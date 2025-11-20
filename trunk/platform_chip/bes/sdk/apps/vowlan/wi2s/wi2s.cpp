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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "hal_wifitsf.h"
#include "audioflinger.h"

#include "vow_dev.h"
#include "vow_session.h"
#include "vow_stream.h"
#include "vowlan_main.h"
#include "wi2s.h"
#include "wui.h"

#define ALIGNED4                        ALIGNED(4)

#define WI2S_STREAM_ID                  AUD_STREAM_ID_1
#if defined(VOWLAN_8_CHANNEL_MODE) && !defined(VOWLAN_SUPPORT_INPUT_BY_I2S)
#define VOW_I2S_CHANNEL_SIZE            (VOW_FRAME_MAX_PAYLOAD_LEN / 8)
#else
#define VOW_I2S_CHANNEL_SIZE            (VOW_FRAME_MAX_PAYLOAD_LEN / 2)
#endif

#ifdef VOWLAN_INPUT_I2S_TYPE_STANDARD
#if defined(VOWLAN_SUPPORT_OUTPUT_BY_USB) && defined(VOWLAN_SUPPORT_INPUT_BY_I2S)
#define WI2S_STREAM_DEVICE AUD_STREAM_USE_I2S0_MASTER
#else
#define WI2S_STREAM_DEVICE AUD_STREAM_USE_I2S0_SLAVE
#endif
#else
#if defined(VOWLAN_SUPPORT_OUTPUT_BY_USB) && defined(VOWLAN_SUPPORT_INPUT_BY_I2S)
#define WI2S_STREAM_DEVICE AUD_STREAM_USE_TDM0_MASTER
#else
#define WI2S_STREAM_DEVICE AUD_STREAM_USE_TDM0_SLAVE
#endif
#endif

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
#define VOW_I2S_RECV_MAX_PAYLOAD_LEN            (VOW_I2S_CHANNEL_SIZE * 2)
#define WI2S_STREAM_DMA_BUFFER_SIZE             (VOWLAN_OUTPUT_DMA_BUFFER_SIZE)
#else
/* 48k/16bit/2ch */
#define VOW_I2S_RECV_MAX_PAYLOAD_LEN            (VOW_I2S_CHANNEL_SIZE * 2 / 4)
#define WI2S_STREAM_DMA_BUFFER_SIZE             (48 * 2 * 2 * VOW_PLAY_INTERVAL * 2)
#endif

static SRAM_HEAP_LOC uint8_t wi2s_stream_dma_buf[WI2S_STREAM_DMA_BUFFER_SIZE] ALIGNED4;

#ifdef __VOW_SPLIT_L_R_CHANNEL__
SRAM_BSS_LOC uint8_t wi2s_ch0[VOW_I2S_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FL */
SRAM_BSS_LOC uint8_t wi2s_ch1[VOW_I2S_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FR */
#endif

static uint8_t *vow_i2s_recv_buffer_new = NULL;
SRAM_BSS_LOC uint8_t vow_i2s_recv_buffer_old[VOW_I2S_RECV_MAX_PAYLOAD_LEN] ALIGNED4;

osSemaphoreDef(i2s_irq_sem);
static osSemaphoreId i2s_irq_sem = NULL;

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
static VOW_AUDIO_CFG rf_out_audio_cfg;
#endif
static VOW_AUDIO_CFG local_out_i2s_audio_cfg;

static int8_t wi2s_tran_vsess_id[VOWLAN_RF_OUTPUT_NUM] = {0};
static int8_t wi2s_main_vsess_id = -1;
static uint8_t g_audio_channel = 2;
static uint32_t i2s_recv_payload_len = 0;
static uint64_t i2s_timestamp = 0;

#ifndef WI2S_AUDIO_INPUT_CODEC_TASK_SZ
#define WI2S_AUDIO_INPUT_CODEC_TASK_SZ      (8 * 1024)
#endif
static void wi2s_audio_input_data_codec(void const *pArg);
osThreadDef(wi2s_audio_input_data_codec, osPriorityAboveNormal, 1, WI2S_AUDIO_INPUT_CODEC_TASK_SZ, "i2s_input_codec_tid");
static osThreadId i2s_input_codec_tid = NULL;

static uint32_t wi2s_stream_dma_callback(uint8_t *buf, uint32_t len)
{
    if (VOW_I2S_RECV_MAX_PAYLOAD_LEN == len) {
        vow_i2s_recv_buffer_new = buf;
#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
        i2s_timestamp = hal_wifitsf_timer_get() + rf_out_audio_cfg.delay;
#else
        i2s_timestamp = hal_wifitsf_timer_get() + local_out_i2s_audio_cfg.delay;
#endif
        osSemaphoreRelease(i2s_irq_sem);
    } else
        TRACE(0, "I2S input error len = %d, match len = %d", len, VOW_I2S_RECV_MAX_PAYLOAD_LEN);

    return 0;
}

static void wi2s_audio_input_ch71_24bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
    //TODO
}

static void wi2s_audio_input_ch71_16bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
    //TODO
}

static void wi2s_audio_input_ch51_24bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
    //TODO
}

static void wi2s_audio_input_ch51_16bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
    //TODO
}

static void wi2s_audio_input_ch2_24bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
    POSSIBLY_UNUSED VOW_SESSION *session = NULL;
    POSSIBLY_UNUSED uint32 i = 0;
    POSSIBLY_UNUSED SceneParam_t *sceneparam = vow_get_scene_param();

#ifdef __VOW_SPLIT_L_R_CHANNEL__
    unsigned char *ptr0 = NULL;
    unsigned char *ptr1 = NULL;
    unsigned char *ptr = (uint8_t*)pData;

    /* transfer format(byte) : |dataL0|dataL1|dataL2|dataR0|dataR1|dataR2|dataL3|dataL4|dataL5|dataR3|...| =>
     * wusb_ch0 : |dataL0|dataL1|dataL2|dataL3|dataL4|dataL5|...|
     * wusb_ch1 : |dataR0|dataR1|dataR2|dataR3|dataR4|dataR5|...|
     */

    ptr0 = (uint8_t *)wi2s_ch0;    /* FL */
    ptr1 = (uint8_t *)wi2s_ch1;    /* FR */
    for (i = 0; i < smpcnt; i++) {
        *ptr0++ = *ptr++;
        *ptr0++ = *ptr++;
        *ptr0++ = *ptr++;
        *ptr1++ = *ptr++;
        *ptr1++ = *ptr++;
        *ptr1++ = *ptr++;
    }

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            return;
        }

        if (session->audio_channel == VOW_CHANNEL_FL)
            vow_session_input(wi2s_tran_vsess_id[i],(uint8_t *)wi2s_ch0, Len, &i2s_timestamp);
    }
#endif

    if (wi2s_main_vsess_id < 0)
        return;
    else
        vow_session_input(wi2s_main_vsess_id, (uint8_t *)wi2s_ch1, Len, &i2s_timestamp);

    return;
#else

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
//            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            return;
        }

        if (session->audio_channel == (VOW_CHANNEL_FL | VOW_CHANNEL_FR))
            vow_session_input(wi2s_tran_vsess_id[i], (uint8_t *)pData, Len * 2, &i2s_timestamp);
    }
#endif

    if (wi2s_main_vsess_id < 0)
        return;
    else
        vow_session_input(wi2s_main_vsess_id, (uint8_t *)pData, Len * 2, &i2s_timestamp);

    return;
#endif
}

static void wi2s_audio_input_ch2_16bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
    POSSIBLY_UNUSED VOW_SESSION *session = NULL;
    POSSIBLY_UNUSED uint32 i = 0;
    POSSIBLY_UNUSED SceneParam_t *sceneparam = vow_get_scene_param();

#ifdef __VOW_SPLIT_L_R_CHANNEL__
    unsigned short *ptr0 = NULL;
    unsigned short *ptr1 = NULL;
    unsigned short *ptr = (uint16_t*)pData;

    /* transfer format(byte) : |dataL0|dataL1|dataR0|dataR1|dataL2|dataL3|...| =>
     * wusb_ch0 : |dataL0|dataL1|dataL2|dataL3|...|
     * wusb_ch1 : |dataR0|dataR1|dataR2|dataR3|...|
     */
    ptr0 = (uint16_t *)wi2s_ch0;    /* FL */
    ptr1 = (uint16_t *)wi2s_ch1;    /* FR */
    for (i = 0; i < smpcnt; i++) {
        *ptr0++ = *ptr++;
        *ptr1++ = *ptr++;
    }

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            return;
        }

        if (session->audio_channel == VOW_CHANNEL_FL)
            vow_session_input(wi2s_tran_vsess_id[i],(uint8_t *)wi2s_ch0, Len, &i2s_timestamp);
    }
#endif

    if (wi2s_main_vsess_id < 0)
        return;
    else
        vow_session_input(wi2s_main_vsess_id, (uint8_t *)wi2s_ch1, Len, &i2s_timestamp);

    return;
#else

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            return;
        }

        if (session->audio_channel == (VOW_CHANNEL_FL | VOW_CHANNEL_FR))
            vow_session_input(wi2s_tran_vsess_id[i], (uint8_t *)pData, Len * 2, &i2s_timestamp);
    }
#endif

    if (wi2s_main_vsess_id < 0)
        return;
    else
        vow_session_input(wi2s_main_vsess_id, (uint8_t *)pData, Len * 2, &i2s_timestamp);

    return;
#endif
}

static void wi2s_audio_input_data_codec(void const *pArg)
{
    uint8_t *pData =NULL ;
    uint32 smpcnt = 0;
    uint32 Len = 0;
    while (1) {
        osSemaphoreWait(i2s_irq_sem, osWaitForever);
        Len = i2s_recv_payload_len;

        /* update tsf */
        vow_update_wifitsf();

        if (vow_wifi_get_ap_status() == 0) {
            //todo
            TRACE(0,"i2s: softap <==> sta link is not online...");
            return;
        }

#if defined(__VOW_SPLIT_L_R_CHANNEL__)
        /* no audio data move */
        pData = vow_i2s_recv_buffer_new;
#else
        /* audio data move */
        vow_memset(vow_i2s_recv_buffer_old, 0 , VOW_I2S_RECV_MAX_PAYLOAD_LEN);
        vow_memcpy(vow_i2s_recv_buffer_old , vow_i2s_recv_buffer_new, VOW_I2S_RECV_MAX_PAYLOAD_LEN);

        pData = vow_i2s_recv_buffer_old;
#endif

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
        switch (rf_out_audio_cfg.sample_depth) {
        case AUD_BITS_16:
            smpcnt = (VOW_I2S_RECV_MAX_PAYLOAD_LEN >> 1); /* all ch sample count */
            break;
        case AUD_BITS_24:
            smpcnt = (VOW_I2S_RECV_MAX_PAYLOAD_LEN >> 2); /* all ch sample count */
            if (g_audio_channel == 2) {
                /* Must convert : 32bit to 24bit */
                uint8_t *buffer_24bit = NULL;
                uint32 Len_32bit = VOW_I2S_RECV_MAX_PAYLOAD_LEN;
                if(vow_stereo_convert_32bit_to_24bit(pData, &Len_32bit, &buffer_24bit) < 0) {
                    TRACE(0, "%s, vow_convert_32bit_to_24bit fail!", __func__);
                    goto wi2s_audio_input_over;
                } else
                    pData = buffer_24bit;
            } else {
                //TODO:
                TRACE(0, "%s, to be added : g_audio_channel %d. sample_depth = %d", __func__, g_audio_channel, rf_out_audio_cfg.sample_depth);
                goto wi2s_audio_input_over;
            }
            break;
        default:
            TRACE(0,"%s invalid depth=%d", __func__, rf_out_audio_cfg.sample_depth);
            goto wi2s_audio_input_over;
        }

        Len /= g_audio_channel;     /* calculate every ch length */
        smpcnt /= g_audio_channel;  /* every ch sample count */

        if (rf_out_audio_cfg.sample_depth == AUD_BITS_16) {
            if (g_audio_channel == 8) {
                wi2s_audio_input_ch71_16bit_data_handle(pData, smpcnt, Len);
            } else if (g_audio_channel == 6) {
                wi2s_audio_input_ch51_16bit_data_handle(pData, smpcnt, Len);
            } else if(g_audio_channel == 2) {
                wi2s_audio_input_ch2_16bit_data_handle(pData, smpcnt, Len);
            } else {
                TRACE(0,"%s sample_depth 16bit. Not supported invalid g_audio_channel=%d for now.", __func__, g_audio_channel);
            }
        } else if (rf_out_audio_cfg.sample_depth == AUD_BITS_24) {
            if (g_audio_channel == 8) {
                wi2s_audio_input_ch71_24bit_data_handle(pData, smpcnt, Len);
            } else if(g_audio_channel == 6) {
                wi2s_audio_input_ch51_24bit_data_handle(pData, smpcnt, Len);
            } else if (g_audio_channel == 2) {
                wi2s_audio_input_ch2_24bit_data_handle(pData, smpcnt, Len);
            } else {
                TRACE(0,"%s sample_depth 24bit. Not supported invalid g_audio_channel=%d for now.", __func__, g_audio_channel);
            }
        } else {
            TRACE(0,"%s Not supported invalid depth=%d for now.", __func__, rf_out_audio_cfg.sample_depth);
        }
#else
        switch (local_out_i2s_audio_cfg.sample_depth) {
        case AUD_BITS_16:
            smpcnt = (VOW_I2S_RECV_MAX_PAYLOAD_LEN >> 1); /* all ch sample count */
            break;
        case AUD_BITS_24:
            smpcnt = (VOW_I2S_RECV_MAX_PAYLOAD_LEN >> 2); /* all ch sample count */
            if (g_audio_channel == 2) {
                /* Must convert : 32bit to 24bit */
                uint8_t *buffer_24bit = NULL;
                uint32 Len_32bit = VOW_I2S_RECV_MAX_PAYLOAD_LEN;
                if(vow_stereo_convert_32bit_to_24bit(pData, &Len_32bit, &buffer_24bit) < 0) {
                    TRACE(0, "%s, vow_convert_32bit_to_24bit fail!", __func__);
                    goto wi2s_audio_input_over;
                } else
                    pData = buffer_24bit;
            } else {
                //TODO:
                TRACE(0, "%s, to be added : g_audio_channel %d. sample_depth = %d", __func__, g_audio_channel, local_out_i2s_audio_cfg.sample_depth);
                goto wi2s_audio_input_over;
            }
            break;
        default:
            TRACE(0,"%s invalid depth=%d", __func__, local_out_i2s_audio_cfg.sample_depth);
            goto wi2s_audio_input_over;
        }

        Len /= g_audio_channel;     /* calculate every ch length */
        smpcnt /= g_audio_channel;  /* every ch sample count */

        if (local_out_i2s_audio_cfg.sample_depth == AUD_BITS_16) {
            if (g_audio_channel == 8) {
                wi2s_audio_input_ch71_16bit_data_handle(pData, smpcnt, Len);
            } else if (g_audio_channel == 6) {
                wi2s_audio_input_ch51_16bit_data_handle(pData, smpcnt, Len);
            } else if(g_audio_channel == 2) {
                wi2s_audio_input_ch2_16bit_data_handle(pData, smpcnt, Len);
            } else {
                TRACE(0,"%s sample_depth 16bit. Not supported invalid g_audio_channel=%d for now.", __func__, g_audio_channel);
            }
        } else if (local_out_i2s_audio_cfg.sample_depth == AUD_BITS_24) {
            if (g_audio_channel == 8) {
                wi2s_audio_input_ch71_24bit_data_handle(pData, smpcnt, Len);
            } else if(g_audio_channel == 6) {
                wi2s_audio_input_ch51_24bit_data_handle(pData, smpcnt, Len);
            } else if (g_audio_channel == 2) {
                wi2s_audio_input_ch2_24bit_data_handle(pData, smpcnt, Len);
            } else {
                TRACE(0,"%s sample_depth 24bit. Not supported invalid g_audio_channel=%d for now.", __func__, g_audio_channel);
            }
        } else {
            TRACE(0,"%s Not supported invalid depth=%d for now.", __func__, local_out_i2s_audio_cfg.sample_depth);
        }
#endif

        wi2s_audio_input_over:
            ;
    }
}

void i2s_codec_task_init(void)
{
    TRACE(0, "enter %s %d", __func__, __LINE__);

    if (i2s_irq_sem == NULL)
        i2s_irq_sem = osSemaphoreCreate(osSemaphore(i2s_irq_sem), 0);

    ASSERT(i2s_irq_sem, "%s failed to create sem.", __func__);

    if (i2s_input_codec_tid == NULL)
        i2s_input_codec_tid = osThreadCreate(osThread(wi2s_audio_input_data_codec), NULL);
    
    ASSERT(i2s_input_codec_tid, "%s failed to create i2s codec task.", __func__);

    TRACE(0, "create i2s codec task = 0x%X", (uint32_t)i2s_input_codec_tid);

    return;
}

void wi2s_session_updata(VOW_DEVICE *vodev, SceneParam_t *sceneparam)
{
#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    uint8_t i = 0;
    VOW_SESSION *session = NULL;
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if ((wi2s_tran_vsess_id[i] != -1) && (session == NULL)) {   /* dev off line -> reconnection */
            wi2s_tran_vsess_id[i] = vow_session_alloc(vodev, &rf_out_audio_cfg, VOW_SOURCE_USB, VOW_DEST_WIFI);
            if (wi2s_tran_vsess_id[i] < 0) {
                TRACE(0, "%s:%d,vow_session_alloc fail, wi2s_tran_vsess_id[%d]=%d", __func__, __LINE__,i, wi2s_tran_vsess_id[i]);
                break;
            }
            TRACE(0, "wi2s_tran_vsess_id[%d]:%d", i, wi2s_tran_vsess_id[i]);
        }
    }
#endif
}

void wi2s_init(VOW_DEVICE *vodev, SceneParam_t *sceneparam)
{
    uint32_t submit_size = 0;
    int i = 0;

    if (!vodev) {
        TRACE(0, "wi2s_init fail, vodev error");
        return;
    }

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    /* net audio session alloc */
    if (sceneparam->output_device & VOW_OUTPUT_WIFI) {
        rf_out_audio_cfg.sample_rate = sceneparam->sample_freq;
        rf_out_audio_cfg.sample_depth = sceneparam->sample_depth;
        rf_out_audio_cfg.delay = sceneparam->play_interval * VOW_BEAT_TIME_NUM;     /* paly retardation */

        rf_out_audio_cfg.mute_state = 0;
        rf_out_audio_cfg.channel_num = sceneparam->play_ch_num;;
        rf_out_audio_cfg.vol = 15;
        rf_out_audio_cfg.burst_size = 1;

        for (i = 0; i < sceneparam->rf_tx_number; i++) {
            wi2s_tran_vsess_id[i] = -1;
            wi2s_tran_vsess_id[i] = vow_session_alloc(vodev, &rf_out_audio_cfg, VOW_SOURCE_IIS, VOW_DEST_WIFI);
            if (wi2s_tran_vsess_id[i] < 0) {
                TRACE(0, "%s:%d, vow_session_alloc fail, wi2s_tran_vsess_id[%d]=%d", __func__, __LINE__, i, wi2s_tran_vsess_id[i]);
                goto FAIL;
            }
            TRACE(0, "wi2s_tran_vsess_id[%d]:%d", i, wi2s_tran_vsess_id[i]);
        }
    }

    /* local audio session alloc */
    if ((sceneparam->output_device & VOW_OUTPUT_IIS) || (sceneparam->output_device & VOW_OUTPUT_DAC)) {
        local_out_i2s_audio_cfg.sample_rate = sceneparam->sample_freq;
        local_out_i2s_audio_cfg.sample_depth = sceneparam->sample_depth;
        local_out_i2s_audio_cfg.delay = sceneparam->play_interval * VOW_BEAT_TIME_NUM;      /* paly retardation */

        local_out_i2s_audio_cfg.mute_state = 0;
        local_out_i2s_audio_cfg.channel_num = sceneparam->play_ch_num;
        local_out_i2s_audio_cfg.vol = 15;
        local_out_i2s_audio_cfg.burst_size = 1;

        if (sceneparam->output_device & VOW_OUTPUT_IIS)
            wi2s_main_vsess_id = vow_session_alloc(vodev, &local_out_i2s_audio_cfg, VOW_SOURCE_IIS, VOW_DEST_IIS);
        else if (sceneparam->output_device & VOW_OUTPUT_DAC)
            wi2s_main_vsess_id = vow_session_alloc(vodev, &local_out_i2s_audio_cfg, VOW_SOURCE_IIS, VOW_DEST_DAC);

        if (wi2s_main_vsess_id < 0) {
            TRACE(0, "%s:%d, vow_session_alloc fail", __func__, __LINE__);
            goto FAIL;
        }
        TRACE(0, "wi2s_main_vsess_id:%d", wi2s_main_vsess_id);
    }
#else
    /* local audio session alloc */
    if (sceneparam->output_device & VOW_OUTPUT_USB) {
        local_out_i2s_audio_cfg.sample_rate = AUD_SAMPRATE_48000;
        local_out_i2s_audio_cfg.sample_depth = AUD_BITS_16;
        local_out_i2s_audio_cfg.delay = sceneparam->play_interval * VOW_BEAT_TIME_NUM;      /* paly retardation */

        local_out_i2s_audio_cfg.mute_state = 0;
        local_out_i2s_audio_cfg.channel_num = sceneparam->play_ch_num;
        local_out_i2s_audio_cfg.vol = 15;
        local_out_i2s_audio_cfg.burst_size = 1;

        wi2s_main_vsess_id = vow_session_alloc(vodev, &local_out_i2s_audio_cfg, VOW_SOURCE_IIS, VOW_DEST_USB);

        if (wi2s_main_vsess_id < 0) {
            TRACE(0, "%s:%d, vow_session_alloc fail", __func__, __LINE__);
            goto FAIL;
        }
        TRACE(0, "wi2s_main_vsess_id:%d", wi2s_main_vsess_id);
    }
#endif

    /* submit_size config */
    submit_size = ((sceneparam->sample_freq / 1000) * (sceneparam->sample_depth / 8) * 2 * \
                sceneparam->play_interval) / 1000;

    /* calculate submit_size */
    submit_size *= (sceneparam->i2s_input_ch_num / 2); /* such as : 6ch = 2ch * 6 / 2 */
    g_audio_channel = sceneparam->i2s_input_ch_num;

    TRACE(0, "%s:%d, submit_size=%d, g_audio_channel=%d", __func__, __LINE__, submit_size, g_audio_channel);

    i2s_recv_payload_len = submit_size;

    /* i2s audio input init */
    i2s_codec_task_init();

    struct AF_STREAM_CONFIG_T stream_cfg;
    vow_memset(&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    stream_cfg.bits = sceneparam->sample_depth;
    stream_cfg.sample_rate = sceneparam->sample_freq;
#else
    stream_cfg.bits = local_out_i2s_audio_cfg.sample_depth;
    stream_cfg.sample_rate = local_out_i2s_audio_cfg.sample_rate;
#endif

    /* for I2S need set channel to 2 */
    stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)sceneparam->i2s_input_ch_num;
    stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);

    stream_cfg.data_ptr = wi2s_stream_dma_buf;
    stream_cfg.data_size = WI2S_STREAM_DMA_BUFFER_SIZE;
    stream_cfg.handler = wi2s_stream_dma_callback;
    stream_cfg.device = WI2S_STREAM_DEVICE;
    stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
    stream_cfg.vol = 15;

    if (stream_cfg.device == AUD_STREAM_USE_TDM0_MASTER ||
        stream_cfg.device == AUD_STREAM_USE_TDM0_SLAVE ||
        stream_cfg.device == AUD_STREAM_USE_TDM1_MASTER ||
        stream_cfg.device == AUD_STREAM_USE_TDM1_SLAVE) {

        stream_cfg.sync_start = false;
        if (stream_cfg.bits == AUD_BITS_24)
            stream_cfg.slot_cycles = AUD_BITS_32;
        else
            stream_cfg.slot_cycles = stream_cfg.bits;

        stream_cfg.fs_cycles = 1;   /* HAL_TDM_FS_CYCLES_1 */
        stream_cfg.align = AUD_DATA_ALIGN_I2S;
        stream_cfg.fs_edge = AUD_FS_FIRST_EDGE_POS;
    }

    af_stream_open(WI2S_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
    return;

FAIL:
#if defined(VOWLAN_SUPPORT_OUTPUT_BY_USB)
    if (sceneparam->output_device & VOW_OUTPUT_WIFI) {
        for (i = 0; i < sceneparam->rf_tx_number; i++) {
            if (wi2s_tran_vsess_id[i] >= 0) {
                vow_session_free(wi2s_tran_vsess_id[i]);
                wi2s_tran_vsess_id[i] = -1;
            }
        }
    }
#endif

    if (wi2s_main_vsess_id >= 0) {
        vow_session_free(wi2s_main_vsess_id);
        wi2s_main_vsess_id = -1;
    }

    return;
}

static bool i2s_capture_start = false;
void vow_i2s_capture_start(void)
{
    if (i2s_capture_start == true)
        return;
    af_stream_start(WI2S_STREAM_ID, AUD_STREAM_CAPTURE);
    i2s_capture_start = true;
}

void vow_i2s_capture_stop(void)
{
    if (i2s_capture_start == false)
        return;
    af_stream_stop(WI2S_STREAM_ID, AUD_STREAM_CAPTURE);
    i2s_capture_start = false;
}