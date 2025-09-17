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
#include "cmsis_os.h"
#include "hal_trace.h"
#include "string.h"

/* for audio */
#include "hw_codec_iir_process.h"
#include "audioflinger.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "audio_tester.h"
#include "cqueue.h"
#include "analog.h"
#include CHIP_SPECIFIC_HDR(reg_analog)
#include <stdlib.h>
#include <math.h>

#ifndef CHIP_BEST1502X
#include "aud_section.h"
#include "anc_process.h"
#endif

#include "hal_dma.h"
#include "hal_aud.h"
#include "hal_i2s.h"
#include CHIP_SPECIFIC_HDR(reg_cmu)


#define CHANNEL_NUM         (2)
#define PLAY_CHANNEL_NUM    (2)

#define SAMPLE_BITS     (24)
#define SAMPLE_BYTES    (SAMPLE_BITS / 8)

#define TX_SAMPLE_RATE  (48000)
#define RX_SAMPLE_RATE  (48000)

#define SINWAVE_TX_SAMPLE_RATE  (48000)
#define SINWAVE_RX_SAMPLE_RATE  (48000)

#define TX_FRAME_LEN    (256)
#define RX_FRAME_LEN    (256)

#define TX_BUF_SIZE   (TX_FRAME_LEN * CHANNEL_NUM * SAMPLE_BYTES * 2)
#define RX_BUF_SIZE   (RX_FRAME_LEN * PLAY_CHANNEL_NUM * SAMPLE_BYTES * 2)

static enum AUD_BITS_T BIT_SEL = 24;

static uint8_t SPK_GAIN = 12 ;
static uint8_t POSSIBLY_UNUSED MIC_GAIN = 12 ;
static uint8_t MIC_CH_MAP = (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
static uint8_t CH_NUM_SEL = 2;

bool mute_flag = 0;

static uint8_t POSSIBLY_UNUSED codec_capture_buf[TX_BUF_SIZE];
static uint8_t POSSIBLY_UNUSED codec_playback_buf[RX_BUF_SIZE];
static uint8_t POSSIBLY_UNUSED loop_buf[RX_BUF_SIZE];

/******************* anc param *******************/
#ifndef CHIP_BEST1502X
static uint8_t pcsuppt_anc_type = ANC_FEEDFORWARD|ANC_FEEDBACK;
static enum AUD_SAMPRATE_T anc_sample_rate=AUD_SAMPRATE_50781;
static struct_anc_cfg g_anc_config=
{
    .anc_cfg_ff_l = {

        .total_gain = 512,

        .iir_bypass_flag=0,
        .iir_counter=2,

        .iir_coef[0].coef_b={131723630,-260565855,130644731},//7k
        .iir_coef[0].coef_a={134217728,-260565855,128150634},
        .iir_coef[1].coef_b={133853574,-267512069,133696049},//1k
        .iir_coef[1].coef_a={134217728,-267512069,133331895},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
    .anc_cfg_ff_r = {
        .total_gain = 0,

        .iir_bypass_flag=0,
        .iir_counter=1,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
#endif


    .anc_cfg_fb_l = {
        .total_gain = 512,

        .iir_bypass_flag=0,
        .iir_counter=1,

        .iir_coef[0].coef_b={131378448,-259182743,130150231},//8k
        .iir_coef[0].coef_a={134217728,-259182743,127310951},
        .iir_coef[1].coef_b={133853574,-267512069,133696049},//1k
        .iir_coef[1].coef_a={134217728,-267512069,133331895},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,

    },
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)

    .anc_cfg_fb_r = {
        .total_gain = 0,

        .iir_bypass_flag=0,
        .iir_counter=1,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
#endif


    .anc_cfg_mc_l = {
        .total_gain = 0,

        .iir_bypass_flag=0,
        .iir_counter=1,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)

    .anc_cfg_mc_r = {
        .total_gain = 0,

        .iir_bypass_flag=0,
        .iir_counter=1,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
#endif
};
#endif

/******************* control pcmbuff access *******************/
static CQueue audio_test_pcm_queue;
static osMutexId audio_test_pcmbuff_mutex_id = NULL;
osMutexDef(audio_test_pcmbuff_mutex);

/*******************adc ana gain*******************/
static const int8_t adc_db[] = { -9, -6, -3, 0, 3, 6, 9, 12, };
static int ana_adc_gain = adc_db[3];
static const int8_t dig_dac_db[] = {-99,-45,-42,-39,-36,-33,-30,-27,-24,-21,-18,-15,-12,-9,-6,-3,0};

#ifndef MAX_ANA_MIC_CH_NUM
#define MAX_ANA_MIC_CH_NUM                  5
#endif

/*******************hardware dac iir eq*******************/
#define EQ_HW_DAC_IIR_LIST_NUM              1
static const IIR_CFG_T audio_eq_hw_dac_iir_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 10,   1000,   0.3},
        {IIR_TYPE_PEAK, -10,  10000,  0.3},
        {IIR_TYPE_PEAK, 0,   0,   0.3},
        {IIR_TYPE_PEAK, 0,   0,   0.3},
        {IIR_TYPE_PEAK, 0,   0,   0.3},
        {IIR_TYPE_PEAK, 0,   0,   0.3},
        {IIR_TYPE_PEAK, 0,   0,   0.3},
        {IIR_TYPE_PEAK, 0,   0,   0.3},
    }
};

static const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list[EQ_HW_DAC_IIR_LIST_NUM]={
    &audio_eq_hw_dac_iir_cfg,
};

int audio_test_pcmbuff_put(uint8_t *buff, uint16_t len)
{
    int status;

    osMutexWait(audio_test_pcmbuff_mutex_id, osWaitForever);
    status = EnCQueue(&audio_test_pcm_queue, buff, len);
    osMutexRelease(audio_test_pcmbuff_mutex_id);

    return status;
}

int audio_test_pcmbuff_get(uint8_t *buff, uint16_t len)
{
    unsigned char *e1 = NULL, *e2 = NULL;
    unsigned int len1 = 0, len2 = 0;
    int status;

    osMutexWait(audio_test_pcmbuff_mutex_id, osWaitForever);
    status = PeekCQueue(&audio_test_pcm_queue, len, &e1, &len1, &e2, &len2);
    if (len==(len1+len2)){
        memcpy(buff,e1,len1);
        memcpy(buff+len1,e2,len2);
        DeCQueue(&audio_test_pcm_queue, 0, len1);
        DeCQueue(&audio_test_pcm_queue, 0, len2);
    }else{
        memset(buff, 0x00, len);
        status = -1;
    }
    osMutexRelease(audio_test_pcmbuff_mutex_id);

    return status;
}

int audio_test_pcmbuff_init(uint8_t *buff, uint16_t len)
{
    if (audio_test_pcmbuff_mutex_id == NULL)
        audio_test_pcmbuff_mutex_id = osMutexCreate((osMutex(audio_test_pcmbuff_mutex)));

    if ((buff == NULL)||(audio_test_pcmbuff_mutex_id == NULL))
        return -1;

    osMutexWait(audio_test_pcmbuff_mutex_id, osWaitForever);
    InitCQueue(&audio_test_pcm_queue, len, buff);
    memset(buff, 0x00, len);
    osMutexRelease(audio_test_pcmbuff_mutex_id);

    return 0;
}
static enum AUDIO_TEST_CACHE_T a2dp_cache_status = AUDIO_TEST_CACHE_QTY;

static uint32_t POSSIBLY_UNUSED addaloop_test_data_come(uint8_t *buf, uint32_t len)
{
    if(mute_flag == 1){
        memset(buf,0,len);
    }
    //TRACE(1, "%s , len = %d", __func__ , len);
    //DUMP16("%d ",buf,16);
    audio_test_pcmbuff_put(buf, len);
    if (a2dp_cache_status == AUDIO_TEST_CACHE_QTY){
        a2dp_cache_status = AUDIO_TEST_CACHE_OK;
    }
    return len;
}


const int16_t test_sin [] = {
#include "1k_2ch_48k_16bit.txt"
};
audio_test_pcmpatten_t pcmpatten;

static uint32_t POSSIBLY_UNUSED addaloop_test_more_data(uint8_t *buf, uint32_t len)
{
    //TRACE(1, "%s , len = %d", __func__ , len);
    //DUMP16("%d ",buf,16);
    if (a2dp_cache_status != AUDIO_TEST_CACHE_QTY){
        audio_test_pcmbuff_get((uint8_t *)buf, len);
    }

    return len;
}

int addaloop_test_output_pcmpatten(audio_test_pcmpatten_t *pcmpatten, uint8_t *buf, uint32_t len)
{
    uint32_t remain_size = len;
    uint32_t curr_size = 0;

    if (remain_size > pcmpatten->len)
    {
        do{
            if (pcmpatten->cuur_buf_pos)
            {
                curr_size = pcmpatten->len-pcmpatten->cuur_buf_pos;
                memcpy(buf,&(pcmpatten->buf[pcmpatten->cuur_buf_pos/2]), curr_size);
                remain_size -= curr_size;
                pcmpatten->cuur_buf_pos = 0;
            }
            else if (remain_size>pcmpatten->len)
            {
                memcpy(buf+curr_size, pcmpatten->buf, pcmpatten->len);
                curr_size += pcmpatten->len;
                remain_size -= pcmpatten->len;
            }
            else
            {
                memcpy(buf+curr_size,pcmpatten->buf, remain_size);
                pcmpatten->cuur_buf_pos = remain_size;
                remain_size = 0;
            }
        }while(remain_size);
    }
    else
    {
        if ((pcmpatten->len - pcmpatten->cuur_buf_pos) >= len)
        {
            memcpy(buf, &(pcmpatten->buf[pcmpatten->cuur_buf_pos/2]),len);
            pcmpatten->cuur_buf_pos += len;
        }
        else
        {
            curr_size = pcmpatten->len-pcmpatten->cuur_buf_pos;
            memcpy(buf, &(pcmpatten->buf[pcmpatten->cuur_buf_pos/2]),curr_size);
            pcmpatten->cuur_buf_pos = len - curr_size;
            memcpy(buf+curr_size, pcmpatten->buf, pcmpatten->cuur_buf_pos);
        }
    }

    return 0;
}

static uint32_t POSSIBLY_UNUSED sinwave_test_more_data(uint8_t *buf, uint32_t len)
{
    //TRACE(1, "%s , len = %d", __func__ , len);
    //DUMP16("%d ",buf,16);
    if (a2dp_cache_status != AUDIO_TEST_CACHE_QTY){
        audio_test_pcmbuff_get((uint8_t *)buf, len);
    }

    addaloop_test_output_pcmpatten(&pcmpatten, buf, len);
    //memset(buf,0x55,len);
    return len;
}

/*analog_adc_gain operation*/
static uint32_t POSSIBLY_UNUSED db_to_adc_gain(int db)
{
    int i;
    uint8_t cnt;
    const int8_t *list;

    list = adc_db;
    cnt = ARRAY_SIZE(adc_db);

    for (i = 0; i < cnt - 1; i++) {
        if (db < list[i + 1]) {
            break;
        }
    }

    if (i == cnt - 1) {
        return i;
    }
    else if (db * 2 < list[i] + list[i + 1]) {
        return i;
    } else {
        return i + 1;
    }
}

static void POSSIBLY_UNUSED analog_aud_set_adc_gain(enum AUD_IO_PATH_T input_path, enum AUD_CHANNEL_MAP_T ch_map, int gain)
{
    int i;
    uint16_t gain_val;
    uint16_t reg;
    uint16_t val;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {

        //MIC_CH0/1/2/4
        if (i == 3){
            continue;
        }
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            gain_val = db_to_adc_gain(gain);
            reg = ANA_REG_61 + 0x10 * i;
            analog_read(reg, &val);
            val = SET_BITFIELD(val, REG_CODEC_ADCA_GAIN_BIT, gain_val);
            analog_write(reg, val);
        }
    }
}

int audio_tester_addaloop(bool on, enum HAL_CMU_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    TRACE_IMM(3,"audio_tester_addaloop work:%d freq:%d", on, freq);

    if (on){
        TRACE_IMM(0,"  on");
        if (freq < HAL_CMU_FREQ_52M) {
            freq = HAL_CMU_FREQ_52M;
        }
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, freq);

        a2dp_cache_status = AUDIO_TEST_CACHE_QTY;

        audio_test_pcmbuff_init(loop_buf, RX_BUF_SIZE);

        TRACE_IMM(0,"  audio_test_pcmbuff_init");
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CH_NUM_SEL;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)TX_SAMPLE_RATE;
        stream_cfg.bits = BIT_SEL;
        stream_cfg.vol = MIC_GAIN;
        stream_cfg.channel_map = MIC_CH_MAP;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = addaloop_test_data_come;
        stream_cfg.data_ptr = codec_capture_buf;
        stream_cfg.data_size = TX_BUF_SIZE;

        TRACE_IMM(2,"codec capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CH_NUM_SEL;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)RX_SAMPLE_RATE;
        stream_cfg.bits = BIT_SEL;
        stream_cfg.vol = SPK_GAIN;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = addaloop_test_more_data;
        stream_cfg.data_ptr = codec_playback_buf;
        stream_cfg.data_size = RX_BUF_SIZE;

        TRACE_IMM(2,"codec playback sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        // Start
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        TRACE(0,"audio_tester_addaloop off");

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    }

    return 0;
}

int audio_tester_sinwaveloop(bool on, enum HAL_CMU_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    TRACE_IMM(3,"audio_tester_sinwaveloop work:%d freq:%d", on, freq);

    pcmpatten.buf = (uint16_t *)test_sin;
    pcmpatten.cuur_buf_pos = 0;
    pcmpatten.len = sizeof(test_sin);

    if (on){
        TRACE_IMM(0,"aytayt  on");
        if (freq < HAL_CMU_FREQ_52M) {
            freq = HAL_CMU_FREQ_52M;
        }
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, freq);

        a2dp_cache_status = AUDIO_TEST_CACHE_QTY;

        audio_test_pcmbuff_init(loop_buf, RX_BUF_SIZE);

        TRACE_IMM(0,"audio_test_pcmbuff_init");
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CHANNEL_NUM;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)SINWAVE_TX_SAMPLE_RATE;
        stream_cfg.bits = BIT_SEL;
        stream_cfg.vol = MIC_GAIN;
        // stream_cfg.channel_map = CH_MAP_SEL;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = addaloop_test_data_come;
        stream_cfg.data_ptr = codec_capture_buf;
        stream_cfg.data_size = TX_BUF_SIZE;

        TRACE_IMM(2,"codec capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)PLAY_CHANNEL_NUM;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)SINWAVE_RX_SAMPLE_RATE;
        stream_cfg.bits = BIT_SEL;
        stream_cfg.vol = SPK_GAIN;
        // stream_cfg.channel_map = CH_MAP_SEL;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = sinwave_test_more_data;
        stream_cfg.data_ptr = codec_playback_buf;
        stream_cfg.data_size = RX_BUF_SIZE;

        TRACE_IMM(2,"codec playback sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        // Start
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        TRACE(0,"audio_tester_sinwaveloop off");

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    }

    return 0;
}

int pdm_tester_function(bool on, enum HAL_CMU_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    TRACE_IMM(3,"pdm_tester_function work:%d freq:%d", on, freq);

    if (on){
        TRACE_IMM(0,"  on");
        if (freq < HAL_CMU_FREQ_52M) {
            freq = HAL_CMU_FREQ_52M;
        }
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, freq);

        a2dp_cache_status = AUDIO_TEST_CACHE_QTY;

        audio_test_pcmbuff_init(loop_buf, RX_BUF_SIZE);

        TRACE_IMM(0,"  audio_test_pcmbuff_init");
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CHANNEL_NUM;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)TX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = MIC_GAIN;
        stream_cfg.channel_map = AUD_CHANNEL_MAP_DIGMIC_CH0|AUD_CHANNEL_MAP_DIGMIC_CH1;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = addaloop_test_data_come;
        stream_cfg.data_ptr = codec_capture_buf;
        stream_cfg.data_size = TX_BUF_SIZE;

        TRACE_IMM(2,"codec capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)PLAY_CHANNEL_NUM;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)RX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = SPK_GAIN;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = addaloop_test_more_data;
        stream_cfg.data_ptr = codec_playback_buf;
        stream_cfg.data_size = RX_BUF_SIZE;

        TRACE_IMM(2,"codec playback sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        // Start
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        TRACE(0,"pdm_tester_function off");

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    }

    return 0;
}

extern uint32_t hal_codec_get_real_sample_rate(enum AUD_SAMPRATE_T rate, bool resamp);
int eq_tester_function(bool on, enum HAL_CMU_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    TRACE_IMM(3,"eq_tester_function work:%d freq:%d", on, freq);

    HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac = NULL;
    const IIR_CFG_T *iir_cfg = audio_eq_hw_dac_iir_cfg_list[0];

    if (on){
        TRACE_IMM(0,"on");
        if (freq < HAL_CMU_FREQ_52M) {
            freq = HAL_CMU_FREQ_52M;
        }
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, freq);

        a2dp_cache_status = AUDIO_TEST_CACHE_QTY;

        audio_test_pcmbuff_init(loop_buf, RX_BUF_SIZE);

        TRACE_IMM(0,"audio_test_pcmbuff_init");
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CH_NUM_SEL;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)TX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = MIC_GAIN;
        stream_cfg.channel_map = MIC_CH_MAP;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = addaloop_test_data_come;
        stream_cfg.data_ptr = codec_capture_buf;
        stream_cfg.data_size = TX_BUF_SIZE;

        TRACE_IMM(2,"codec capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CH_NUM_SEL;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)RX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = SPK_GAIN;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = addaloop_test_more_data;
        stream_cfg.data_ptr = codec_playback_buf;
        stream_cfg.data_size = RX_BUF_SIZE;

        TRACE_IMM(2,"codec playback sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        //EQ start
        hw_codec_iir_open(stream_cfg.sample_rate, HW_CODEC_IIR_DAC, (AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1));
        TRACE_IMM(0,"%s real sample rate = %d", __func__, hal_codec_get_real_sample_rate(stream_cfg.sample_rate,1));
        hw_iir_cfg_dac = hw_codec_iir_get_cfg(hal_codec_get_real_sample_rate(stream_cfg.sample_rate,1), iir_cfg);
        hw_codec_iir_set_cfg(hw_iir_cfg_dac, stream_cfg.sample_rate, HW_CODEC_IIR_DAC);

        // Start
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        TRACE(0,"eq_tester_function off");

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    }

    return 0;
}

//#define I2S_DATA_OUTPUT
/*↓↓↓↓↓↓↓↓           _audio_i2s_test_           ↓↓↓↓↓↓↓↓*/
int i2s_tester_function(bool on, enum HAL_CMU_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    TRACE_IMM(3,"i2s_tester_function work:%d freq:%d", on, freq);

    hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_SYS);
    TRACE_IMM(0,"hal_cmu_pll_enable");
    hal_cmu_sys_select_pll(HAL_CMU_PLL_BB);
    TRACE_IMM(0,"hal_cmu_sys_select_pll");
    hal_sysfreq_req(HAL_SYSFREQ_USER_FIR, HAL_CMU_FREQ_52M);//x4 canot run with 104M
    TRACE_IMM(0,"hal_sysfreq_req");

    if (on){
        TRACE_IMM(0,"  on");
        if (freq < HAL_CMU_FREQ_52M) {
            freq = HAL_CMU_FREQ_52M;
        }
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, freq);

        a2dp_cache_status = AUDIO_TEST_CACHE_QTY;

        audio_test_pcmbuff_init(loop_buf, RX_BUF_SIZE);

        TRACE_IMM(0,"  audio_test_pcmbuff_init");
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CH_NUM_SEL;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)TX_SAMPLE_RATE;
        stream_cfg.bits = BIT_SEL;
        stream_cfg.vol = MIC_GAIN;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = addaloop_test_data_come;
        stream_cfg.data_ptr = codec_capture_buf;
        stream_cfg.data_size = TX_BUF_SIZE;

        TRACE_IMM(2,"codec capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CH_NUM_SEL;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)RX_SAMPLE_RATE;
        stream_cfg.bits = BIT_SEL;
        stream_cfg.vol = SPK_GAIN;
        //stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = addaloop_test_more_data;
        stream_cfg.data_ptr = codec_playback_buf;
        stream_cfg.data_size = RX_BUF_SIZE;

        TRACE_IMM(2,"codec playback sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        // Start
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        TRACE(0,"i2s_tester_function off");

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    }

    return 0;
}
/*↑↑↑↑↑↑↑↑           _audio_i2s_test_           ↑↑↑↑↑↑↑↑*/

/*↓↓↓↓↓↓↓↓           _audio_anc_           ↓↓↓↓↓↓↓↓*/
#ifndef CHIP_BEST1502X
static struct_anc_cfg *get_anc_config(void)
{
    return &g_anc_config;
}

static void POSSIBLY_UNUSED anc_function(bool on)
{
    enum AUD_SAMPRATE_T play_rate, capture_rate;
    struct_anc_cfg *anccfg = get_anc_config();

    play_rate = anc_sample_rate;
    capture_rate = anc_sample_rate;

    TRACE(1,"%s pcsuppt_anc_type:0x%x", __func__,pcsuppt_anc_type);

    TRACE(0,"[%s]:AUD_SECTION_STRUCT_VERSION=%d!",__func__,AUD_SECTION_STRUCT_VERSION);

    if(on){
        if(pcsuppt_anc_type & ANC_FEEDFORWARD)
        {
            af_anc_open(ANC_FEEDFORWARD, play_rate, capture_rate, NULL);
            anc_open(ANC_FEEDFORWARD);

            anc_enable();
            // anc_select_coef(play_rate,cur_coef_index,ANC_FEEDFORWARD,ANC_GAIN_DELAY);

            anc_set_cfg(anccfg,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
            anc_set_gain(512,512,ANC_FEEDFORWARD);
        }

        if(pcsuppt_anc_type & ANC_FEEDBACK)
        {
            af_anc_open(ANC_FEEDBACK, play_rate, capture_rate, NULL);
            anc_open(ANC_FEEDBACK);

            anc_enable();
            anc_set_cfg(anccfg,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
            anc_set_gain(512,512,ANC_FEEDBACK);
            // anc_select_coef(play_rate,cur_coef_index,ANC_FEEDBACK,ANC_GAIN_DELAY);
        }
    }
    else if(false){
        if(pcsuppt_anc_type & ANC_FEEDFORWARD)
        {
            anc_disable();
            anc_close(ANC_FEEDFORWARD);
            af_anc_close(ANC_FEEDFORWARD);
        }

        if(pcsuppt_anc_type & ANC_FEEDBACK)
        {
            anc_disable();
            anc_close(ANC_FEEDBACK);
            af_anc_close(ANC_FEEDBACK);
        }
    }

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    hw_codec_iir_open(AUD_SAMPRATE_48000, HW_CODEC_IIR_DAC, CODEC_OUTPUT_DEV);
#endif

    return;
}


void anc_tester_function(bool on)
{
    if(on){
        TRACE(1,"%s", __func__);

        analog_debug_config_anc_calib_mode(true);

        hal_audma_open();

        hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_SYS);
        hal_cmu_sys_select_pll(HAL_CMU_PLL_BB);
        hal_sysfreq_req(HAL_SYSFREQ_USER_FIR, HAL_CMU_FREQ_52M);

        anc_function(true);
    }
    else{
        anc_function(false);
    }
}
#endif
/*↑↑↑↑↑↑↑↑           _audio_anc_           ↑↑↑↑↑↑↑↑*/

void audio_tester(const char **params)
{
    int cnt=0;
    int operation;
    char *pre_func[]={
        "adda_loop",
        "sin_wave",
        "mute",
        "pdm",
        "eq",
        "i2s",
#ifndef CHIP_BEST1502X
        "anc",
#endif
    };
    char *param[7];


/*[UART,audio_tester CMD on/off ch0/1 mic1/2/3/4/5 bit16/24 spkgain micgain]*/
/*param[0] CMD:adda_loop/sin_wave/mute/eq/i2s/anc */
/*param[1] on/off*/
/*param[2] ch_num 0/1 */
/*param[3] mic_map 0/1/2/3/4 */
/*param[4] bit16/bit24*/
/*param[5] analog_adc_gain 0-7*/
/*param[6] dig_mic_gain:range 1-16*/

    char *p = strtok((char *)params, " ,");
    param[cnt] = p;
    if(p){
        param[cnt] = p;
        while((p = strtok(NULL, " ,")) && p){
            cnt++;
            param[cnt] = p;
        }
    }

    if(!strstr(param[2],"x"))
    {
        CH_NUM_SEL = atoi(param[2]);
    }
    TRACE(0,"CH_NUM_SEL = %d",CH_NUM_SEL);

    if(!strstr(param[3],"x"))
    {
        MIC_CH_MAP = 0;
        if(CH_NUM_SEL == 2){
            MIC_CH_MAP = (AUD_CHANNEL_MAP_CH0 << atoi(strtok(param[3], " /")));
            MIC_CH_MAP |= (AUD_CHANNEL_MAP_CH0 << atoi(strtok(NULL, " /")));
        }
        else{
            MIC_CH_MAP = (AUD_CHANNEL_MAP_CH0 << atoi(strtok(param[3], " /")));
        }
    }
    TRACE(0,"MIC_CH_MAP = %d",MIC_CH_MAP);

    if(!strstr(param[4],"x"))
    {
        BIT_SEL=atoi(param[4]);
    }
    TRACE(0,"BIT_SEL = %d",BIT_SEL);

    if(!strstr(param[5],"x"))
    {
    ana_adc_gain = adc_db[atoi(param[5])];
    }
    TRACE(0,"ANA_ADC_GAIN = %d dB",ana_adc_gain);

    if(!strstr(param[6],"x"))
    {
    SPK_GAIN=atoi(param[6]);
    }
    TRACE(0,"SPK_DIG_GAIN = %d dB",dig_dac_db[SPK_GAIN]);

    for(operation=0;operation<ARRAY_SIZE(pre_func);operation++){
        if(!strcmp(param[0],pre_func[operation])){
            switch(operation)
            {
                case AUDIO_TESTER_ADDALOOP:
                    if(!strcmp(param[1],"on")){
                        audio_tester_addaloop(true, HAL_CMU_FREQ_32K );
                    }
                    if(!strcmp(param[1],"off")){
                        audio_tester_addaloop(false, HAL_CMU_FREQ_32K );
                    }
                    break;
                case AUDIO_TESTER_SINWAVE:
                    if(!strcmp(param[1],"on")){
                        audio_tester_sinwaveloop(true, HAL_CMU_FREQ_32K);
                    }
                    if(!strcmp(param[1],"off")){
                        audio_tester_sinwaveloop(false, HAL_CMU_FREQ_32K);
                    }
                    break;
                case AUDIO_TESTER_MUTE:
                    if(!strcmp(param[1],"on")){
                        mute_flag = 1;
                        audio_tester_addaloop(true, HAL_CMU_FREQ_32K);
                    }
                    if(!strcmp(param[1],"off")){
                        mute_flag = 0;
                        audio_tester_addaloop(false, HAL_CMU_FREQ_32K);
                    }
                    break;
                case AUDIO_TESTER_PDM:
                    if(!strcmp(param[1],"on")){
                        pdm_tester_function(true, HAL_CMU_FREQ_32K);
                    }
                    if(!strcmp(param[1],"off")){
                        pdm_tester_function(false, HAL_CMU_FREQ_32K);
                    }
                    break;
                case AUDIO_TESTER_EQ:
                    if(!strcmp(param[1],"on")){
                        eq_tester_function(true, HAL_CMU_FREQ_32K);
                    }
                    if(!strcmp(param[1],"off")){
                        eq_tester_function(false, HAL_CMU_FREQ_32K);
                    }
                    break;
                case AUDIO_TESTER_I2S:
                    if(!strcmp(param[1],"on")){
                        i2s_tester_function(true, HAL_CMU_FREQ_32K);
                    }
                    if(!strcmp(param[1],"off")){
                        i2s_tester_function(false, HAL_CMU_FREQ_32K);
                    }
                    break;
#ifndef CHIP_BEST1502X
                case AUDIO_TESTER_ANC:
                    if(!strcmp(param[1],"on")){
                        anc_tester_function(true);
                    }
                    if(!strcmp(param[1],"off")){
                        anc_tester_function(false);
                    }
                    break;
#endif
                default:
                    break;
            }
        }
    }

    if(!strcmp(param[1],"on")){
        //write ana_adc_gain
        analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, AUD_CHANNEL_MAP_CH1, ana_adc_gain);
    }

    return ;
}


