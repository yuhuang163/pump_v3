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
#include <string.h>
#include "hal_trace.h"
#include "audio_dump.h"
#include "bt_sco_codec.h"
#include "speech_cfg.h"
#include "lc3_process.h"
#define SCO_LC3_BES_PLC_ON
#define LC3_AUDIO_DUMP
#if defined(SCO_LC3_BES_PLC_ON)
#include "sbcplc.h"
#endif

/**
 * NOTE:
 *  1. This example shows lc3_swb encoder, decoder and PLC
 **/
// ------------------------------ Macro ------------------------------
#if defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST2300)
#define LC3_MUTE_PATTERN (0x00)
#else
#define LC3_MUTE_PATTERN (0x55)
#endif

// ------------------------------ Codec Common ------------------------------
#define CODEC_PACKET_SIZE           (58)
#define SCO_PACKET_SIZE             (60)
#define SCO_HEAD_SIZE               (2)
#define LC3_PCM_FRAME_SAMPLES       (240)
#define LC3_PCM_FRAME_SIZE          (LC3_PCM_FRAME_SAMPLES * sizeof(int16_t))

#define SCO_PACKET_HEADER           (0x01)
#define SCO_PACKET_SEQ_NUM          (4)
static const uint8_t g_sco_packet_seq[SCO_PACKET_SEQ_NUM] = {0x08, 0x38, 0xc8, 0xf8};

typedef struct {
    uint8_t     header;
    uint8_t     seq;
    uint8_t     payload[CODEC_PACKET_SIZE];
} sco_lc3_packet_t;

// ------------------------------ Encoder ------------------------------
static LC3_Enc_Info *lc3_encoder = NULL;
static uint8_t g_encoder_seq_index = 0;

// ------------------------------ Decoder ------------------------------
static LC3_Dec_Info *lc3_decoder = NULL;


static uint8_t *lc3_buf_all = NULL;
static uint32_t lc3_find_first_sync = 0;
static uint32_t lc3_offset = 0;
static uint32_t next_frame_flag = 0;

// ------------------------------ Memory Alloc ------------------------------

static void *lc3_alloc(void *pool, unsigned size){
    TRACE(0, "lc3_alloc(%d)", size);
    void *data = speech_calloc(1, size);
    return data;
}
static void lc3_free(void *pool,void *data){
    TRACE(0, "lc3_free");
    speech_free(data);
    return ;
}

// ------------------------------ PLC ------------------------------
#if defined(SCO_LC3_BES_PLC_ON)
static struct PLC_State *lc3_plc_state1 = NULL;
static float *lc3_cos_buf = NULL;
static int lc3_packet_len = 0;
static int lc3_smooth_len = 0;
#endif

// ------------------------------ Dump ------------------------------


/**
 * Dynamic malloc RAM from speech_heap:
 *  1. PLC:
 *  2. This module: 8k Bytes
 **/
static int32_t bt_sco_codec_lc3_swb_init(uint32_t sample_rate)
{
    TRACE(0, "[%s] SCO: LC3...", __func__);

    ASSERT(SCO_PACKET_SIZE == sizeof(sco_lc3_packet_t),
        "[%s] SCO_PACKET_SIZE != sizeof(sco_lc3_packet_t)", __func__);

    // Add 1 to ensure it never be out of bounds when lc3_offset is 1
    lc3_buf_all = (uint8_t *)speech_calloc(SCO_PACKET_SIZE * 3 + 1, sizeof(uint8_t));    // 181 Bytes
    memset(lc3_buf_all, 0x0, (SCO_PACKET_SIZE * 3 + 1) * sizeof(uint8_t));

    int err = LC3_API_OK;

    lc3_encoder = (LC3_Enc_Info *)speech_calloc(1, sizeof(LC3_Enc_Info));
    g_encoder_seq_index = 0;

    lc3_encoder->bitrate       = 0;
    lc3_encoder->is_interlaced = 1;
    lc3_encoder->epmode        = LC3_API_EP_OFF;
    lc3_encoder->sample_rate   = sample_rate;
    lc3_encoder->channels      = 1;
    lc3_encoder->bitwidth      = 16;
    lc3_encoder->bitalign      = (lc3_encoder->bitwidth==24)?32:lc3_encoder->bitwidth;
    lc3_encoder->frame_dms     = 75;
    lc3_encoder->frame_size    = CODEC_PACKET_SIZE;
    lc3_encoder->pool          = 0;
    lc3_encoder->cb_alloc      = lc3_alloc;
    lc3_encoder->cb_free       = lc3_free;
    lc3_encoder->cb_overlay    = 0;

    err = lc3_api_encoder_init(lc3_encoder);
    ASSERT(LC3_API_OK == err, "%s/%d err %d", __func__,__LINE__, err);
    ASSERT(lc3_encoder->instance, "%s/%d", __func__, __LINE__);
    ASSERT(lc3_encoder->cb_encode_interlaced, "%s/%d", __func__, __LINE__);
    ASSERT(lc3_encoder->frame_samples, "%s/%d", __func__, __LINE__);
    ASSERT(lc3_encoder->frame_size, "%s/%d", __func__, __LINE__);

    /* Print info */
    TRACE(0, "[lc3]");
    TRACE(0, "Encoder size:     %d", lc3_encoder->instance_size);
    TRACE(0, "Scratch size:     %d", lc3_encoder->scratch_size);
    TRACE(0, "Sample rate:      %d", lc3_encoder->sample_rate);
    TRACE(0, "Channels:         %d", lc3_encoder->channels);
    TRACE(0, "Frame samples:    %d", lc3_encoder->frame_samples);
    TRACE(0, "Frame length:     %d", lc3_encoder->frame_size);
    TRACE(0, "Output format:    %d bits", lc3_encoder->bitwidth);
    TRACE(0, "Output format:    %d bits align", lc3_encoder->bitalign);
    TRACE(0, "Target bitrate:   %d", lc3_encoder->bitrate);


    lc3_decoder = (LC3_Dec_Info *)speech_calloc(1, sizeof(LC3_Dec_Info));

    lc3_decoder->sample_rate       = sample_rate;
    lc3_decoder->channels          = 1;
    lc3_decoder->bitwidth          = 16;
    lc3_decoder->bitalign          = (lc3_decoder->bitwidth==24)? 32 : lc3_decoder->bitwidth;
    lc3_decoder->frame_dms         = 75;
    lc3_decoder->frame_size        = CODEC_PACKET_SIZE;
    lc3_decoder->bitrate           = 0;
    lc3_decoder->plcMeth           = LC3_API_PLC_ADVANCED;
    lc3_decoder->epmode            = LC3_API_EP_OFF;
    lc3_decoder->is_interlaced     = 1;
    lc3_decoder->pool              = 0;
    lc3_decoder->cb_alloc          = lc3_alloc;
    lc3_decoder->cb_free           = lc3_free;
    lc3_decoder->cb_overlay        = 0;

    err = lc3_api_decoder_init(lc3_decoder);

    ASSERT(LC3_API_OK == err, "%s err %d", __func__, err);

    /* Print info */
    TRACE(0, "[lc3]");
    TRACE(0, "Decoder size:     %d", lc3_decoder->instance_size);
    TRACE(0, "Scratch size:     %d", lc3_decoder->scratch_size);
    TRACE(0, "Bits:             %d", lc3_decoder->bitwidth);
    TRACE(0, "Bits align:       %d", lc3_decoder->bitalign);
    TRACE(0, "Sample rate:      %d", lc3_decoder->sample_rate);
    TRACE(0, "Channels:         %d", lc3_decoder->channels);
    TRACE(0, "Frame samples:    %d", lc3_decoder->frame_samples);
    TRACE(0, "Frame length:     %d", lc3_decoder->frame_size);
    TRACE(0, "PLC mode:         %d", lc3_decoder->plcMeth);
    TRACE(0, "Bitrate:          %d", lc3_decoder->bitrate);

#if defined(SCO_LC3_BES_PLC_ON)
    lc3_packet_len = lc3_decoder->sample_rate * lc3_decoder->frame_dms / 10000 * 2;
    lc3_smooth_len = lc3_packet_len * 4;
    lc3_plc_state1 = (struct PLC_State *)lc3_alloc(0, sizeof(struct PLC_State));
    a2dp_plc_lc3_init(lc3_plc_state1, A2DP_PLC_CODEC_TYPE_LC3, lc3_packet_len / 2);
    lc3_cos_buf = (float *)lc3_alloc(0, lc3_smooth_len * sizeof(float));
    cos_generate(lc3_cos_buf, lc3_smooth_len, lc3_packet_len);
#endif

    return 0;
}

static int32_t bt_sco_codec_lc3_swb_deinit(void)
{
    TRACE(0, "[%s] ...", __func__);

    if(lc3_encoder->cb_uninit)
    {
        lc3_encoder->cb_uninit(lc3_encoder);
    }

    if(lc3_decoder->cb_uninit)
    {
        lc3_decoder->cb_uninit(lc3_decoder);
    }

    speech_free(lc3_buf_all);
    speech_free(lc3_encoder);
    speech_free(lc3_decoder);

#if defined(SCO_LC3_BES_PLC_ON)
    lc3_free(0, lc3_plc_state1);
    lc3_free(0, lc3_cos_buf);
#endif

    return 0;
}

// lc3_size = 120, pcm_size = 480
static int32_t bt_sco_codec_lc3_swb_encoder(uint8_t *lc3_buf, uint32_t lc3_size, uint8_t *pcm_buf, uint32_t pcm_size)
{

    ASSERT(lc3_size / SCO_PACKET_SIZE == pcm_size / LC3_PCM_FRAME_SIZE,
        "[%s] lc3_size = %d, pcm_size = %d", __func__, lc3_size, pcm_size);

    uint32_t loop_cnt = lc3_size / SCO_PACKET_SIZE;
    sco_lc3_packet_t *sco_packet_ptr = (sco_lc3_packet_t *)lc3_buf;

    int err = LC3_API_OK;
    // Parser packet
    for (uint32_t i = 0; i < loop_cnt; i++) {

        sco_packet_ptr->header  = SCO_PACKET_HEADER;
        sco_packet_ptr->seq     = g_sco_packet_seq[g_encoder_seq_index];

        err = lc3_encoder->cb_encode_interlaced(lc3_encoder, lc3_encoder->scratch,
            pcm_buf, sco_packet_ptr->payload, CODEC_PACKET_SIZE);

        if(err){
            TRACE(0, "lc3 enc err:%d\n",err);
        }
        sco_packet_ptr++;
        pcm_buf += LC3_PCM_FRAME_SIZE;
        g_encoder_seq_index = (g_encoder_seq_index + 1) % SCO_PACKET_SEQ_NUM;
    }

    return 0;
}

static int32_t bt_sco_codec_lc3_swb_decoder(uint8_t *lc3_buf, uint32_t lc3_size, uint8_t *pcm_buf, uint32_t pcm_size)
{
    ASSERT(lc3_size / SCO_PACKET_SIZE == pcm_size / LC3_PCM_FRAME_SIZE,
        "[%s] lc3_size = %d, pcm_size = %d", __func__, lc3_size, pcm_size);

    uint32_t lc3_offset_lowdelay = 0;
    uint32_t i, j;
    uint8_t *lc3_buffer = (uint8_t *)lc3_buf;
    int frame_flag[6];  // 1: bad frame; 0:good frame;
    uint32_t frame_counter = 0;
    uint32_t lc3_offset_total = 0;
    int lc3_offset_drift[6] = {0};

    short *dec_pcm_buf = (short *)pcm_buf;
    uint8_t dec_lc3_buf[CODEC_PACKET_SIZE] = {0};

    for(i = 0; i < lc3_size; i++) {
        lc3_buf_all[i + SCO_PACKET_SIZE] = lc3_buffer[i];
    }

    for(j = 0; j < lc3_size / SCO_PACKET_SIZE; j++) {
        frame_flag[j] = 0;
    }

    if (lc3_find_first_sync == 0) {
        for (i =0; i<lc3_size; i++) {
            if((lc3_buf_all[i] == 0x01) && ((lc3_buf_all[i+1] & 0x0f) == 0x08))
                break;
        }

        TRACE(1, "sync......:%d", i);

        if (i < lc3_size) {
            lc3_find_first_sync = 1;
            lc3_offset = i % SCO_PACKET_SIZE;
            lc3_offset_total = i;
        } else {
            for (j = 0; j < lc3_size / SCO_PACKET_SIZE; j++) {
                frame_flag[j] = 1;
            }
        }
    }

    if (lc3_find_first_sync == 1) {
        int value = 0;
        uint8_t headerm1 = 0;
        uint8_t header0 = 0;
        uint8_t header1 = 0;
        uint8_t header2 = 0;

        if(lc3_offset == 0 || lc3_offset == 1) {
            lc3_offset_lowdelay = lc3_offset + 60;
        } else {
            lc3_offset_lowdelay = lc3_offset;
        }

        // check again
        for (j = 0; j < lc3_size / SCO_PACKET_SIZE; j++) {
            if (next_frame_flag == 1) {
                next_frame_flag = 0;
                frame_flag[j] = 1;
                continue;
            }

            if (lc3_offset_lowdelay == 0 && j == 0) {
                headerm1 = 0x01;
            } else {
                headerm1 = lc3_buf_all[lc3_offset_lowdelay + j*SCO_PACKET_SIZE - 1];
            }

            header0 = lc3_buf_all[lc3_offset_lowdelay + j*SCO_PACKET_SIZE];
            header1 = lc3_buf_all[lc3_offset_lowdelay + j*SCO_PACKET_SIZE + 1];
            header2 = lc3_buf_all[lc3_offset_lowdelay + j*SCO_PACKET_SIZE + 2];

            if ((headerm1 == 0x01) && ((header0 & 0x0f) == 0x08)) {
                frame_flag[j] = 0;
                // It seems that offset is reduced by 1
                lc3_offset_drift[j] = -1;
                TRACE(1, "[%s] lc3_offset is reduced by 1", __FUNCTION__);
            } else if ((header0 == 0x01) && ((header1 & 0x0f) == 0x08)) {
                frame_flag[j] = 0;
            } else if ((header1 == 0x01) && ((header2 & 0x0f) == 0x08)) {
                frame_flag[j] = 0;
                lc3_offset_drift[j] = 1;
                TRACE(1, "[%s] lc3_offset is increased by 1", __FUNCTION__);
            } else {
                if ((header0 == LC3_MUTE_PATTERN) && ((header1 & 0x0f) == (LC3_MUTE_PATTERN & 0x0f))) {
                    frame_flag[j] = 1;
                } else {
                    if((lc3_offset_lowdelay + j*SCO_PACKET_SIZE) >= lc3_offset_total) {
                        frame_flag[j] = 3;
                    } else {
                        frame_flag[j] = 1;
                    }
                }
            }
        }

        for(j = 0; j < lc3_size / SCO_PACKET_SIZE; j++) {
            value = value | frame_flag[j];
        }

        // abnormal lc3 packet.
        if (value > 1) {
            lc3_find_first_sync = 0;
        }
    }

    while ( (frame_counter < lc3_size / SCO_PACKET_SIZE) &&
            (frame_counter < pcm_size / LC3_PCM_FRAME_SIZE)) {
        uint32_t start_idx = 0;
        if (lc3_offset_lowdelay == 0 && lc3_offset_drift[frame_counter] == -1) {
            start_idx = 1;
            dec_lc3_buf[0] = 0x01;
        }
        for(i = start_idx; i < CODEC_PACKET_SIZE; i++)
        {
            uint32_t offs = lc3_offset_drift[frame_counter] + frame_counter*SCO_PACKET_SIZE + SCO_HEAD_SIZE;
            dec_lc3_buf[i] = lc3_buf_all[i+lc3_offset_lowdelay+offs];
        }

        uint32_t bfi_ext = frame_flag[frame_counter] ? 1 : 0;

        int32_t err = LC3_API_OK;
        err = lc3_decoder->cb_decode_interlaced(lc3_decoder, lc3_decoder->scratch, dec_lc3_buf,
            CODEC_PACKET_SIZE, dec_pcm_buf, bfi_ext);
        if (err)
        {
            TRACE(0, "lc3 dec err:%d", err);
            bfi_ext = 1;
        }
#if defined(SCO_LC3_BES_PLC_ON)
        if (bfi_ext)
        {
            a2dp_plc_bad_frame(lc3_plc_state1, (short *)dec_pcm_buf, (short *)dec_pcm_buf, lc3_cos_buf, lc3_packet_len, 1, 0);
        }
        else {
            a2dp_plc_good_frame(lc3_plc_state1, (short *)dec_pcm_buf, (short *)dec_pcm_buf, lc3_cos_buf, lc3_packet_len, 1, 0);
        }
#endif

        dec_pcm_buf = dec_pcm_buf + LC3_PCM_FRAME_SAMPLES;
        frame_counter++;
    }

    for(i = 0; i < SCO_PACKET_SIZE; i++) {
        lc3_buf_all[i] = lc3_buf_all[i + lc3_size];
    }

    return 0;
}

const bt_sco_codec_t bt_sco_codec_lc3_swb = {
    .init       = bt_sco_codec_lc3_swb_init,
    .deinit     = bt_sco_codec_lc3_swb_deinit,
    .encoder    = bt_sco_codec_lc3_swb_encoder,
    .decoder    = bt_sco_codec_lc3_swb_decoder
};
