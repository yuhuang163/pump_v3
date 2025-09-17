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
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "cmsis_os.h"
#include "audioflinger.h"
#include <stdio.h>
#include <stdlib.h>
#include "smf_api.h"
#include "smf_debug.h"
#include "smf_demuxer_sbc.h"
#include "smf_codec_sbc.h"
#include "SmfExitDestroy.h"
#include "audio_dump.h"
#include "app_utils.h"
#include "app_overlay.h"
#include "kfifo.h"
#include "bth_a2dp_sink.h"
#include "heap_api.h"
#include "sbc_header.h"

using namespace smf;

#define CODEC_PP_BUFF_SIZE (512*5*2/2)
#define SBC_PACKET_LEN (586)
#define SMF_HEAP_SIZE  (32*1024)

osThreadId a2dp_sink_player_tid = NULL;
static void a2dp_sink_player_thread(const void *arg);
osThreadDef(a2dp_sink_player_thread, osPriorityHigh, 1, (1024*32), "a2dp_sink_player");

#define A2DP_SINK_PLAYER_MAILBOX_MAX (16)

static osMailQId a2dp_sink_player_mq_id;
osMailQDef (a2dp_sink_player_mailbox, A2DP_SINK_PLAYER_MAILBOX_MAX, a2dp_sink_msg_t);

static uint8_t codec_playe_data[CODEC_PP_BUFF_SIZE];
static uint8_t a2dp_packet_cache[CODEC_PP_BUFF_SIZE/2];
static uint8_t a2dp_pcm_cache[CODEC_PP_BUFF_SIZE];

static void *sbc_dec;
static void a2dp_sink_smf_init(void);
static bool a2dp_sink_decoder_open(void);

static struct kfifo *p_current_kfifo;
static void a2dp_sink_sbc_decode(uint8_t *pData, uint32_t in_len, uint8_t *pPcm, uint32_t *out_len);

void merge_stereo_to_mono_16bits(int16_t *src_buf, int16_t *dst_buf,  uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; i+=2)
    {
        dst_buf[i] = (src_buf[i]>>1) + (src_buf[i+1]>>1);
        dst_buf[i+1] = dst_buf[i];
    }
}

static void player_copy_track_two_to_one_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t dst_len)
{
    for (uint32_t i = 0; i < dst_len; i++)
    {
        dst_buf[i] = src_buf[i*2];
    }
}


static uint32_t a2dp_sink_more_data(uint8_t *buf, uint32_t len)
{
    uint32_t dec_out_len;

#ifdef AUDIO_DEBUG
    audio_dump_clear_up();
    audio_dump_add_channel_data_from_multi_channels(0, buf, len/2, 1, 0);
    audio_dump_run();
#endif

    a2dp_sink_sbc_decode(a2dp_packet_cache, SBC_PACKET_LEN, a2dp_pcm_cache, &dec_out_len);
    //TRACE(0, "%s need %d out %d", __func__, len , dec_out_len);

    merge_stereo_to_mono_16bits((int16_t*)a2dp_pcm_cache, (int16_t*)a2dp_pcm_cache, dec_out_len/sizeof(int16_t));

    player_copy_track_two_to_one_16bits((int16_t*)buf, (int16_t*)a2dp_pcm_cache, len/sizeof(int16_t));

    kfifo_get(p_current_kfifo, a2dp_packet_cache, SBC_PACKET_LEN);

    return 0;
}

static int a2dp_sink_player_start(uint8_t codec_type)
{
    struct AF_STREAM_CONFIG_T stream_cfg;

    TRACE(0, "%s", __func__);
    app_sysfreq_req(APP_SYSFREQ_USER_BT_A2DP, APP_SYSFREQ_208M);
    a2dp_sink_smf_init();
    //a2dp_sink_demuxer_open();
    a2dp_sink_decoder_open();

    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.sample_rate = AUD_SAMPRATE_44100;
    stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0);
    stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
    stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
    stream_cfg.vol = 12;
    stream_cfg.data_ptr = codec_playe_data;
    stream_cfg.data_size = CODEC_PP_BUFF_SIZE;
    stream_cfg.handler = a2dp_sink_more_data;
    af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

    af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#ifdef AUDIO_DEBUG
    audio_dump_init(CODEC_PP_BUFF_SIZE/4, 2, 1);
#endif
    return 0;
}

static int a2dp_sink_player_stop(void)
{
    TRACE(0, "%s", __func__);
    app_sysfreq_req(APP_SYSFREQ_USER_BT_A2DP, APP_SYSFREQ_32K);
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    return 0;
}

static int a2dp_sink_player_set_volume(uint8_t vol)
{
    af_stream_set_chan_vol(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1), vol/(100/TGT_VOLUME_LEVEL_MAX));
    return 0;
}

static int a2dp_sink_player_mailbox_get(a2dp_sink_msg_t** msg_p)
{
    osEvent evt;

    evt = osMailGet(a2dp_sink_player_mq_id, osWaitForever);

    if (evt.status == osEventMail)
    {
        *msg_p = (a2dp_sink_msg_t *)evt.value.p;
        return 0;
    }
    return -1;
}

static int a2dp_sink_player_mailbox_free(a2dp_sink_msg_t* msg_p)
{
    osStatus status;

    status = osMailFree(a2dp_sink_player_mq_id, msg_p);
    return (int)status;
}


static int a2dp_sink_player_mailbox_init(void)
{

    a2dp_sink_player_mq_id = osMailCreate(osMailQ(a2dp_sink_player_mailbox), NULL);
    if (a2dp_sink_player_mq_id == NULL)
    {
        TRACE(0,"Failed to Create a2dp_sink_player_mailbox\n");
        return -1;
    }
    return 0;
}

static int a2dp_sink_player_mailbox_put(a2dp_sink_msg_t* msg_src)
{
    a2dp_sink_msg_t *msg_p = NULL;
    osStatus status;

    msg_p = (a2dp_sink_msg_t*)osMailAlloc(a2dp_sink_player_mq_id , 0);

    if(!msg_p){
        return -1;
    }
    memcpy((void*)msg_p, (void*)msg_src, sizeof(a2dp_sink_msg_t));
    status = osMailPut(a2dp_sink_player_mq_id, msg_p);

    return (int)status;
}
static void a2dp_sink_player_thread(const void *arg)
{
    a2dp_sink_msg_t  *p_msg = NULL;

    while(1)
    {
        a2dp_sink_player_mailbox_get(&p_msg);

        switch(p_msg->msg_type)
        {
            case A2DP_SINK_MSG_START:
            {
                osDelay(300);
                p_current_kfifo = (struct kfifo *)p_msg->param.onoff_param.kfifo_addr;
                kfifo_get(p_current_kfifo, a2dp_packet_cache, SBC_PACKET_LEN);
                a2dp_sink_player_start(p_msg->param.onoff_param.codec_type);
            }
            break;
            case A2DP_SINK_MSG_VOL_SET:
            {
                a2dp_sink_player_set_volume(p_msg->param.vol_set.volume_level);
            }
            break;
            case A2DP_SINK_MSG_STOP:
            {
                a2dp_sink_player_stop();
            }
            break;
            default:
            break;
        }
        a2dp_sink_player_mailbox_free(p_msg);
    }
}

extern "C" void a2dp_sink_player_init(void)
{
    a2dp_sink_player_mailbox_init();
    app_bth_a2dp_sink_msg_send_cb_register(a2dp_sink_player_mailbox_put);
    if (a2dp_sink_player_tid == NULL) {
        a2dp_sink_player_tid = osThreadCreate(osThread(a2dp_sink_player_thread), NULL);
    }
}

//log output callback;
static int logOutput(const char* log, int size) {
    printf("%s", log);
    return size;
}

static uint8_t smf_heap[SMF_HEAP_SIZE];

static void a2dp_sink_smf_init(void)
{
    //init smf
    smf_init();
    //[Optional]redirect log output callback
    smf_log_redirect1(logOutput);

    med_heap_init(smf_heap, SMF_HEAP_SIZE);

    //[Optional]configure static memory pool
    smf_register_globle_pool_with_callback(med_malloc, med_free);
    smf_register_decoder_pool_with_callback(med_malloc, med_free);
    //smf_register_encoder_pool_with_callback(malloc, free);
    //register file system
    //smf_io_file_register();//register std file system
    //register element
    smf_sbc_decoder_register();
    //smf_sbc_demuxer_register();

    const char* supporteds[32];
    int n= smf_supported_decoder(supporteds,32);
    const char** supported = supporteds;
    dbgTestPXL("smf audio decoder supported list:%d",n);
    while(1){
        if(!*supported)break;
        dbgPrintf("\t support %s\n", *supported);
        supported++;
    }
}


static bool a2dp_sink_decoder_open(void)
{
    //decoder
    sbc_dec = smf_create_decoder("sbc");
    dbgPrintf("create decoder:0x%x", (uint32_t)sbc_dec);
    returnIfErrC(false, !sbc_dec);
    ExitDestroy dec_(sbc_dec);//to smf_destroy(dec) when exit.
    //alloc a openparams
    auto decPara = (smf_sbc_dec_open_param_t*)smf_alloc_open_param(sbc_dec);
    dbgPrintf("open param:0x%x", (uint32_t)decPara);
    returnIfErrC(false, !decPara);
    ExitFree decPara_(sbc_dec, decPara);//to smf_destroy(demux) when exit.
    //fill openParams
    //nothing
    //open
    dbgPrintf("\r\ndec smf open...\r\n");
    returnIfErrC(false, !smf_open(sbc_dec, decPara));
    dec_.Reset();
    dbgPrintf("\r\ndec reset doen\r\n");
    return true;
}

static void a2dp_sink_sbc_decode(uint8_t *pData, uint32_t in_len, uint8_t *pPcm, uint32_t *out_len)
{
    //processing ...
    smf_frame_t fvlc;
    smf_frame_t fpcm;
    uint32_t read_inx = 0;
    uint32_t out_inx = 0;
    uint8_t *ptr;
    smf_sbc_header_t* head;
    uint8_t _channels, _numSubBands, _bitPool, _numBlocks;
    uint32_t _fsize;


    memset(&fvlc, 0, sizeof(fvlc));
    memset(&fpcm, 0, sizeof(fpcm));
    while(read_inx < in_len)
    {
        ptr = pData+read_inx;
        while(*ptr != 0x9c)
        {
            ptr++;
            read_inx++;
        }
        head = (smf_sbc_header_t*)ptr;
        _channels = _tbl_chs[head->ch];
        _numSubBands = _tbl_sbs[head->subbands];
        _numBlocks = _tbl_blks[head->blocks];
        _bitPool = head->bitPool;

        int bits=4*8;///header
        if(SBC_CHNL_MODE_JOINT_STEREO == head->ch){
            bits+=_numSubBands;
        }
        //scale factor
        bits+=_channels*_numSubBands*4;
        //data
        bits+=_numBlocks * _bitPool;
        //
        _fsize = (bits+7)>>3;


        fvlc.buff = pData+read_inx;
        fvlc.size = _fsize;
        fpcm.buff = pPcm+out_inx;

        if (!smf_decode(sbc_dec, &fvlc, &fpcm)) {
            dbgTestPXL("sbc decoder error!");
        }
        read_inx += _fsize;
        out_inx += fpcm.size;
        fpcm.size = 0;
    }

    *out_len = out_inx;
    //printf error
    smf_print_error(sbc_dec);
}