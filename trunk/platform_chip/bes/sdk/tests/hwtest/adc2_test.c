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
#ifdef ADC2_TEST
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_aud.h"
#include "hal_dma.h"
#include "hal_key.h"
#include "string.h"
#include "audioflinger.h"

#ifndef CODEC_SADC_VOL
#define CODEC_SADC_VOL 7
#endif

//#define ADC_LOW_DELAY_TEST

/*
 * for 2700BP socket, only DAC ch0, mic1, mic5 are available
 * the CHIP_2700BP_SOCKET should be defined
 */
//#define CHIP_2700BP_SOCKET

#define CAPTURE_VERBOSE  0 //0, 1, 2
#define PLAYBACK_VERBOSE 0 //0, 1, 2

#ifdef ADC_LOW_DELAY_TEST
#if (CAPTURE_VERBOSE != 0)
#error "cannot output capture trace for low delay test !!"
#endif
#if (PLAYBACK_VERBOSE != 0)
#error "cannot output playback trace for low delay test !!"
#endif
#endif

#ifdef ADC_LOW_DELAY_TEST
#define ADDA_SAMPLE_RATE                384000
#else
#define ADDA_SAMPLE_RATE                48000
#endif

#ifdef ADC_LOW_DELAY_TEST
#define CODEC_ADC1_TEST
#define CODEC_PLAY_IDX                  1 // 1: CODEC1 ADC; 2: CODEC2 ADC
#else
#define CODEC_ADC1_TEST
#define CODEC_ADC2_TEST

#ifdef CODEC_ADC2_DUMP_DATA
#define CODEC_PLAY_IDX                  2 // 1: CODEC1 ADC; 2: CODEC2 ADC
#else
#define CODEC_PLAY_IDX                  2 // 1: CODEC1 ADC; 2: CODEC2 ADC
#endif
#endif

#define CODEC_PLAY_CH_L_ADC_IDX         0 // DAC L: ADC CH 0 -- MIC1

#ifdef CHIP_2700BP_SOCKET
#define CODEC_PLAY_CH_R_ADC_IDX         0 // DAC R: ADC CH 0 -- MIC1
#else /* !defined(CHIP_2700BP_SOCKET) */
#define CODEC_PLAY_CH_R_ADC_IDX         1 // DAC R: ADC CH 0 -- MIC2
#endif

#ifdef ADC_LOW_DELAY_TEST
#define CODEC_ADC1_CHAN_NUM             2 // CODEC1 ADC chan number
#define CODEC_ADC2_CHAN_NUM             2 // CODEC2 ADC chan number
#else
#define CODEC_ADC1_CHAN_NUM             7 // CODEC1 ADC chan number
#define CODEC_ADC2_CHAN_NUM             5 // CODEC2 ADC chan number
#endif

#define ADDA_SAMPLE_SIZE                2  //2: 16bits, 4: 24bits or 32bits
#define ADDA_SAMPLE_BITS                24 //32: 32bits, 24: 24bits

#if (ADDA_SAMPLE_SIZE == 4)
#if (ADDA_SAMPLE_BITS != 24) && (ADDA_SAMPLE_BITS != 32)
#error "ADDA_SAMPLE_BITS not matched with ADDA_SAMPLE_SIZE !"
#endif
#endif

#define ADDA_PLAYBACK_CHAN              2
#define ADDA_CAPTURE1_CHAN              (CODEC_ADC1_CHAN_NUM)
#define ADDA_CAPTURE2_CHAN              (CODEC_ADC2_CHAN_NUM)

#if (CODEC_PLAY_IDX == 1)
#if (ADDA_PLAYBACK_CHAN == 2)
#if (CODEC_ADC1_CHAN_NUM < 2)
#error "CODEC_ADC1_CHAN_NUM too small !!"
#endif
#elif (ADDA_PLAYBACK_CHAN == 1)
#if (CODEC_ADC1_CHAN_NUM < 1)
#error "CODEC_ADC1_CHAN_NUM too small !!"
#endif
#else
#error "Invalid ADDA_PLAYBACK_CHAN !!"
#endif

#elif (CODEC_PLAY_IDX == 2)
#if (ADDA_PLAYBACK_CHAN == 2)
#if (CODEC_ADC2_CHAN_NUM < 2)
#error "CODEC_ADC2_CHAN_NUM too small !!"
#endif
#elif (ADDA_PLAYBACK_CHAN == 1)
#if (CODEC_ADC2_CHAN_NUM < 1)
#error "CODEC_ADC2_CHAN_NUM too small !!"
#endif
#else
#error "Invalid ADDA_PLAYBACK_CHAN !!"
#endif

#else
#error "Invalid CDOEC_PLAY_IDX !!"
#endif

#define RATE_TO_SIZE(n)                 (((n) + (1000 - 1)) / 1000)

#define ADDA_PLAYBACK_FRAME_SIZE        (RATE_TO_SIZE(ADDA_SAMPLE_RATE) * ADDA_SAMPLE_SIZE * ADDA_PLAYBACK_CHAN)
#define ADDA_CAPTURE1_FRAME_SIZE        (RATE_TO_SIZE(ADDA_SAMPLE_RATE) * ADDA_SAMPLE_SIZE * ADDA_CAPTURE1_CHAN)
#define ADDA_CAPTURE2_FRAME_SIZE        (RATE_TO_SIZE(ADDA_SAMPLE_RATE) * ADDA_SAMPLE_SIZE * ADDA_CAPTURE2_CHAN)

#define PLAYBACK_FRAME_NUM              10
#define CAPTURE_FRAME_NUM               20

#define ADDA_NON_EXP_ALIGN(val, exp)    (((val) + ((exp) - 1)) / (exp) * (exp))
#define BUFF_ALIGN                      (4 * 4)

#ifdef ADC_LOW_DELAY_TEST
#define PLAYBACK_LOW_DELAY_SAMPLE_NUM 8
#define CAPTURE_LOW_DELAY_SAMPLE_NUM  8*1
#define PLAYBACK_SIZE                   (PLAYBACK_LOW_DELAY_SAMPLE_NUM*ADDA_SAMPLE_SIZE*ADDA_CAPTURE1_CHAN)
#define CAPTURE1_SIZE                   (CAPTURE_LOW_DELAY_SAMPLE_NUM*ADDA_SAMPLE_SIZE*ADDA_CAPTURE1_CHAN)
#else
#define PLAYBACK_SIZE                   ADDA_NON_EXP_ALIGN(ADDA_PLAYBACK_FRAME_SIZE * PLAYBACK_FRAME_NUM, BUFF_ALIGN)
#define CAPTURE1_SIZE                   ADDA_NON_EXP_ALIGN(ADDA_CAPTURE1_FRAME_SIZE * CAPTURE_FRAME_NUM, BUFF_ALIGN)
#endif

#define CAPTURE2_SIZE                   ADDA_NON_EXP_ALIGN(ADDA_CAPTURE2_FRAME_SIZE * CAPTURE_FRAME_NUM, BUFF_ALIGN)

#define ALIGNED4                        ALIGNED(4)

#if (CODEC_PLAY_IDX == 2)
#ifndef CODEC_ADC2_TEST
#error "CODEC_ADC2_TEST should be defined for ADC2"
#endif
#endif

static uint8_t ALIGNED4 adda_playback_buf[PLAYBACK_SIZE];

static uint8_t ALIGNED4 adda_capture_buf1[CAPTURE1_SIZE];
static uint8_t ALIGNED4 adda_capture_buf2[CAPTURE2_SIZE];

static uint32_t cap_rpos;
static uint32_t cap_wpos;

static bool samp_data_24bit = true;

static uint32_t play_chan_l_idx = CODEC_PLAY_CH_L_ADC_IDX;
static uint32_t play_chan_r_idx = CODEC_PLAY_CH_R_ADC_IDX;

static bool loop_started;
static bool loop_next_state;

static enum AUD_BITS_T sample_size_to_enum(uint32_t size)
{
    if (size == 2) {
        return AUD_BITS_16;
    } else if (size == 4) {
        if (ADDA_SAMPLE_BITS == 24) {
            return AUD_BITS_24;
        } else {
            return AUD_BITS_32;
        }
    } else {
        ASSERT(false, "%s: Invalid sample size: %u", __FUNCTION__, size);
    }

    return 0;
}

static enum AUD_CHANNEL_NUM_T chan_num_to_enum(uint32_t num)
{
    if (num == 2) {
        return AUD_CHANNEL_NUM_2;
    } else if (num == 1) {
        return AUD_CHANNEL_NUM_1;
    } else {
        ASSERT(false, "%s: Invalid channel num: %u", __FUNCTION__, num);
    }

    return 0;
}

void aud_data_dump(const char *s, uint8_t *buf, uint32_t len,
    uint32_t samp_size, uint32_t chan, uint32_t dump_cnt)
{
#define BSIZE 8
    uint32_t i;

    TRACE(0, "%s: dump buf=%x, len=%d, size=%d, ---------", s, (int)buf, len, samp_size);

    if (samp_size == 2) {
        int16_t *ps = (int16_t *)buf;

        for (i = 0; i < dump_cnt/BSIZE; i++) {
            TRACE(0, "%d %d %d %d %d %d %d %d",
                ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7]);
            ps += BSIZE;
        }
    } else {
        int32_t *ps = (int32_t *)buf;

        for (i = 0; i < dump_cnt/BSIZE; i++) {
            TRACE(0, "%d %d %d %d %d %d %d %d",
                ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7]);
            ps += BSIZE;
        }
    }

    TRACE(0, "%s: dump end------------", s);
}

static uint32_t adda_data_playback1(uint8_t *buf, uint32_t len)
{
#ifdef DA_MUTE_APP
    static uint32_t cnt;

    if (++cnt >= 2000) {
        TRACE(0, "[%X] playback: cnt=%u", hal_sys_timer_get(), cnt);
        cnt = 0;
    }
#else
#if (PLAYBACK_VERBOSE > 0)
    uint32_t cur_time;
#endif
    uint32_t avail;

    uint32_t play_chan_num;
    uint32_t cap_chan_num;
    uint32_t cap_buf_size;
    uint8_t *cap_buf_ptr;

    uint32_t play_samp_size = ADDA_SAMPLE_SIZE;

    if (CODEC_PLAY_IDX == 1) {
        cap_buf_ptr  = adda_capture_buf1;
        cap_buf_size = sizeof(adda_capture_buf1);
        cap_chan_num = ADDA_CAPTURE1_CHAN;
    } else {
        cap_buf_ptr  = adda_capture_buf2;
        cap_buf_size = sizeof(adda_capture_buf2);
        cap_chan_num = ADDA_CAPTURE2_CHAN;
    }
    play_chan_num = ADDA_PLAYBACK_CHAN;

#if (PLAYBACK_VERBOSE > 0)
    cur_time = TICKS_TO_MS(hal_sys_timer_get());
#endif

    if (cap_wpos >= cap_rpos) {
        avail = cap_wpos - cap_rpos;
    } else {
        avail = cap_buf_size + cap_wpos - cap_rpos;
    }

    if (avail * play_chan_num / cap_chan_num >= len) {
        if (play_samp_size == 2) {
            // 16bit
            int16_t *src     = (int16_t *)(cap_buf_ptr + cap_rpos);
            int16_t *src_end = (int16_t *)(cap_buf_ptr + cap_buf_size);
            int16_t *dst     = (int16_t *)(buf);
            int16_t *dst_end = (int16_t *)(buf + len);

            while (dst < dst_end) {
                if (src == src_end) {
                    src = (int16_t *)cap_buf_ptr;
                }
#if (ADDA_PLAYBACK_CHAN == 2)
                dst[0] = src[play_chan_l_idx];
                dst[1] = src[play_chan_r_idx];
#elif (ADDA_PLAYBACK_CHAN == 1)
                dst[0] = src[play_chan_l_idx];
#endif
                src += cap_chan_num;
                dst += play_chan_num;
            }
        } else if (play_samp_size == 4) {
            // 32bit or 24bit
            int32_t *src     = (int32_t *)(cap_buf_ptr + cap_rpos);
            int32_t *src_end = (int32_t *)(cap_buf_ptr + cap_buf_size);
            int32_t *dst     = (int32_t *)(buf);
            int32_t *dst_end = (int32_t *)(buf + len);

            while (dst < dst_end) {
                if (src == src_end) {
                    src = (int32_t *)cap_buf_ptr;
                }
#if (ADDA_PLAYBACK_CHAN == 2)
                dst[0] = src[play_chan_l_idx];
                if (cap_chan_num >= 2) {
                       dst[1] = src[play_chan_r_idx];
                }
#elif (ADDA_PLAYBACK_CHAN == 1)
                dst[0] = src[play_chan_l_idx];
#endif
                src += cap_chan_num;
                dst += play_chan_num;
            }
        }
        // update read pos
        cap_rpos += len * cap_chan_num / play_chan_num;
        if (cap_rpos >= cap_buf_size) {
            cap_rpos -= cap_buf_size;
        }
    } else {
        memset(buf, 0, len);
    }

#if (PLAYBACK_VERBOSE > 0)
    TRACE(5,"[%8u] playback: len=%4u wpos=%4u rpos=%4u avail=%4u", cur_time, len, cap_wpos, cap_rpos, avail);
#endif

#endif
    return 0;
}

static uint32_t adda_data_capture1(uint8_t *buf, uint32_t len)
{
#if (CAPTURE_VERBOSE > 0)
    uint32_t cur_time;

    cur_time = TICKS_TO_MS(hal_sys_timer_get());
#endif

#if (CAPTURE_VERBOSE > 1)
    aud_data_dump("capture1", buf, len, 2, 2, 48);
#endif

#if (CODEC_PLAY_IDX == 1)
    cap_wpos += len;
    if (cap_wpos >= sizeof(adda_capture_buf1)) {
        cap_wpos = 0;
    }
#endif

#if (CAPTURE_VERBOSE > 0)
    TRACE(4,"[%8u] capture1: len=%4u wpos=%4u rpos=%4u", cur_time, len, cap_wpos, cap_rpos);
#endif
    return 0;
}

static uint32_t adda_data_capture2(uint8_t *buf, uint32_t len)
{
#if (CAPTURE_VERBOSE > 0)
    uint32_t cur_time;

    cur_time = TICKS_TO_MS(hal_sys_timer_get());
#endif

#if (CAPTURE_VERBOSE > 1)
    aud_data_dump("capture2", buf, len, 2, 2, 48);
#endif

#if (CODEC_PLAY_IDX == 2)
    cap_wpos += len;
    if (cap_wpos >= sizeof(adda_capture_buf2)) {
        cap_wpos = 0;
    }
#endif

#if (CAPTURE_VERBOSE > 0)
    TRACE(4,"[%8u] capture2: len=%4u wpos=%4u rpos=%4u", cur_time, len, cap_wpos, cap_rpos);
#endif
    return 0;
}

int stream_open_capture1(void)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;
    enum AUD_CHANNEL_NUM_T chan_num;

    TRACE(0, "====> %s: ", __func__);

    chan_num = (enum AUD_CHANNEL_NUM_T)ADDA_CAPTURE1_CHAN;

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.sample_rate = ADDA_SAMPLE_RATE;
    stream_cfg.bits = sample_size_to_enum(ADDA_SAMPLE_SIZE);
    stream_cfg.channel_num = chan_num;

    switch (chan_num) {
    case 1:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
        break;
    case 2:
#ifdef CHIP_2700BP_SOCKET
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH4;
#else
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH3;
#endif
        break;
    case 3:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2;
        break;
    case 4:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH3;
        break;
    case 5:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH3
            | AUD_CHANNEL_MAP_CH4;
        break;
    case 6:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH3
            | AUD_CHANNEL_MAP_CH4
            | AUD_CHANNEL_MAP_CH5;
        break;
    case 7:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH3
            | AUD_CHANNEL_MAP_CH4
            | AUD_CHANNEL_MAP_CH5
            | AUD_CHANNEL_MAP_ECMIC_CH0;
        break;
    case 8:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH3
            | AUD_CHANNEL_MAP_CH4
            | AUD_CHANNEL_MAP_CH5
            | AUD_CHANNEL_MAP_ECMIC_CH0
            | AUD_CHANNEL_MAP_ECMIC_CH1;
        break;
    default:
        ASSERT(false, "%s: invalid chan num %d", __func__, chan_num);
        break;
    }

    stream_cfg.device    = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.vol       = CODEC_SADC_VOL;
    stream_cfg.handler   = adda_data_capture1;
    stream_cfg.io_path   = AUD_INPUT_PATH_MAINMIC;
    stream_cfg.data_ptr  = adda_capture_buf1;
    stream_cfg.data_size = sizeof(adda_capture_buf1);

    TRACE(3, "====> capture1: ptr=%x, size=%d", (int)stream_cfg.data_ptr, (uint32_t)stream_cfg.data_size);
    TRACE(3,"[%s] sample_rate: %d,bits=%d", __func__, stream_cfg.sample_rate, stream_cfg.bits);

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open catpure failed: %d", ret);
    return ret;
}

int stream_close_capture1(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "%s: af_stream_start close failed: %d", __func__, ret);

    return ret;
}

int stream_start_capture1(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "%s: af_stream_start catpure failed: %d", __func__, ret);

    return ret;
}

int stream_stop_capture1(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "%s: af_stream_start catpure failed: %d", __func__, ret);

    return ret;
}

int stream_open_capture2(void)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;
    enum AUD_CHANNEL_NUM_T chan_num;

    TRACE(0, "====> %s: ", __func__);

    chan_num = (enum AUD_CHANNEL_NUM_T)ADDA_CAPTURE2_CHAN;

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.sample_rate = ADDA_SAMPLE_RATE;
    stream_cfg.bits = sample_size_to_enum(ADDA_SAMPLE_SIZE);
    stream_cfg.channel_num = chan_num;

    switch (chan_num) {
    case 1:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
        break;
    case 2:
#ifdef CHIP_2700BP_SOCKET
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH4;
#else
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1;
#endif
        break;
    case 3:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2;
        break;
    case 4:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH3;
        break;
    case 5:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH4
            | AUD_CHANNEL_MAP_ECMIC_CH1;
        break;
    case 6:
        stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0
            | AUD_CHANNEL_MAP_CH1
            | AUD_CHANNEL_MAP_CH2
            | AUD_CHANNEL_MAP_CH3
            | AUD_CHANNEL_MAP_CH4
            | AUD_CHANNEL_MAP_CH5;
        break;
    default:
        ASSERT(false, "%s: invalid chan num %d", __func__, chan_num);
        break;
    }

    stream_cfg.device    = AUD_STREAM_USE_INT_CODEC2;
    stream_cfg.vol       = CODEC_SADC_VOL;
    stream_cfg.handler   = adda_data_capture2;
    stream_cfg.io_path   = AUD_INPUT_PATH_MAINMIC;
    stream_cfg.data_ptr  = adda_capture_buf2;
    stream_cfg.data_size = sizeof(adda_capture_buf2);

    TRACE(3, "====> capture2: ptr=%x, size=%d", (int)stream_cfg.data_ptr, (uint32_t)stream_cfg.data_size);
    TRACE(3,"[%s] sample_rate: %d,bits=%d", __func__, stream_cfg.sample_rate, stream_cfg.bits);

    ret = af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "%s: af_stream_open catpure failed: %d", __func__, ret);
    return ret;
}

int stream_close_capture2(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "%s: af_stream_start catpure failed: %d", __func__, ret);

    return ret;
}

int stream_start_capture2(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "%s: af_stream_start catpure failed: %d", __func__, ret);

    return ret;
}

int stream_stop_capture2(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "%s: af_stream_start catpure failed: %d", __func__, ret);

    return ret;
}

#ifdef ADC_LOW_DELAY_TEST

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP)
#define DYNAMIC_AUDIO_BUFFER_COUNT
#endif

#define DYNAMIC_AUDIO_BUFFER_COUNT

#ifdef DYNAMIC_AUDIO_BUFFER_COUNT
#define MAX_AUDIO_BUFFER_COUNT          16
#define MIN_AUDIO_BUFFER_COUNT          2
#define AUDIO_BUFFER_COUNT              (role->dma_desc_cnt)
#else
#define MAX_AUDIO_BUFFER_COUNT          4
#define AUDIO_BUFFER_COUNT              MAX_AUDIO_BUFFER_COUNT
#if (AUDIO_BUFFER_COUNT & 0x1)
#error "AUDIO_BUFFER_COUNT must be an even number"
#endif
#endif

struct af_stream_ctl_t{
    enum AF_PP_T pp_index;      //pingpong operate
    uint8_t pp_cnt;             //use to count the lost signals
    uint8_t status;             //status machine
    enum AUD_STREAM_USE_DEVICE_T use_device;

    uint32_t hdlr_intvl_ticks;
    uint32_t prev_hdlr_time;
    bool first_hdlr_proc;
    bool intvl_check_en;
};

struct af_stream_cfg_t {
    //used inside
    struct af_stream_ctl_t ctl;

    //dma buf parameters, RAM can be alloced in different way
    uint8_t *dma_buf_ptr;
    uint32_t dma_buf_size;

    //store stream cfg parameters
    struct AF_STREAM_CONFIG_T cfg;

    //dma cfg parameters
#ifdef DYNAMIC_AUDIO_BUFFER_COUNT
    uint8_t dma_desc_cnt;
#endif
    struct HAL_DMA_DESC_T dma_desc[MAX_AUDIO_BUFFER_COUNT];
    struct HAL_DMA_CH_CFG_T dma_cfg;

    //callback function
    AF_STREAM_HANDLER_T handler;
};

struct af_stream_cfg_t *af_get_stream_role(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

static AF_STREAM_HANDLER_T audio_data_handler[AUD_STREAM_NUM];

void audio_dma_irq_handler(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    uint8_t *buf;
    uint32_t len;
    uint32_t hw_pos, dma_addr;
    uint32_t pp_index = PP_PING;
    struct af_stream_cfg_t *role = NULL;

    role = af_get_stream_role(id, stream);
    dma_addr = af_stream_get_cur_dma_addr(id, stream);
    hw_pos = dma_addr - (uint32_t)(role->dma_buf_ptr);
    if (hw_pos < role->dma_buf_size / 2) {
        pp_index = PP_PANG;
    } else if (hw_pos < role->dma_buf_size) {
        pp_index = PP_PING;
    }
    if (pp_index == PP_PING) {
        buf = role->dma_buf_ptr;
    } else {
        buf = role->dma_buf_ptr + role->dma_buf_size / 2;
    }
    len = role->dma_buf_size / 2;

    audio_data_handler[stream](buf, len);
}

void audio_dma_handler_init(void)
{
    TRACE(0, "====> %s: ", __func__);

    audio_data_handler[AUD_STREAM_PLAYBACK] = adda_data_playback1;
#ifdef CODEC_ADC1_TEST
    audio_data_handler[AUD_STREAM_CAPTURE] = adda_data_capture1;
#elif defined(CODEC_ADC2_TEST)
    audio_data_handler[AUD_STREAM_CAPTURE] = adda_data_capture2;
#else
#error "ADC1 or ADC2 should be opened for low delay test !!"
#endif
    af_set_irq_notification(audio_dma_irq_handler);
}
#endif

int stream_open_playback1(void)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    TRACE(0, "====> %s: ", __func__);

    TRACE(0, "playback1 stream: playback ADC-%d CH[%d] stream",
        CODEC_PLAY_IDX, CODEC_PLAY_CH_L_ADC_IDX);

    cap_rpos = 0;
    cap_wpos = 0;
    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits        = sample_size_to_enum(ADDA_SAMPLE_SIZE);
    stream_cfg.channel_num = chan_num_to_enum(ADDA_PLAYBACK_CHAN);
    stream_cfg.sample_rate = ADDA_SAMPLE_RATE;
    stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.vol         = 17;
    stream_cfg.handler     = adda_data_playback1;
    stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
    stream_cfg.data_ptr    = adda_playback_buf;
    stream_cfg.data_size   = sizeof(adda_playback_buf);

    if (stream_cfg.bits == AUD_BITS_24) {
        samp_data_24bit = true;
    } else {
        samp_data_24bit = false;
    }
    TRACE(3, "====> playback1: ptr=%x, size=%d", (int)stream_cfg.data_ptr, (uint32_t)stream_cfg.data_size);
    TRACE(3,"[%s] playback sample_rate: %d, bits = %d", __func__, stream_cfg.sample_rate, stream_cfg.bits);

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

#ifdef ADC_LOW_DELAY_TEST
    audio_dma_handler_init();
#endif
    return ret;
}

int stream_close_playback1(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_close playback failed: %d", ret);

    return ret;
}

int stream_start_playback1(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);

    return ret;
}

int stream_stop_playback1(void)
{
    int ret = 0;

    TRACE(0, "====> %s: ", __func__);

    ret = af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_stop playback failed: %d", ret);

    return ret;
}

////////////////////////////// adda loop ////////////////////////////

static void adda_loop_start(void)
{
    TRACE(0, "====> %s: ", __func__);

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_208M);

    TRACE(0, "CPU freq=%d", hal_sys_timer_calc_cpu_freq(5,0));

    stream_open_playback1();

#ifdef CODEC_ADC1_TEST
    stream_open_capture1();
#endif
#ifdef CODEC_ADC2_TEST
    stream_open_capture2();
#endif

    stream_start_playback1();

#ifdef CODEC_ADC1_TEST
    stream_start_capture1();
#endif
#ifdef CODEC_ADC2_TEST
    stream_start_capture2();
#endif
#if 1
    // codec use osc x4 clock
    uint32_t val = *(volatile uint32_t *)0x40300074;
    val &= ~(1<<17); //disable x2
    val |= (1<<16);  //enable x4
    *(volatile uint32_t *)0x40300074 = val;
#endif
}

static void adda_loop_stop(void)
{

    TRACE(0, "====> %s: ", __func__);

#ifdef CODEC_ADC1_TEST
    stream_stop_capture1();
#endif
#ifdef CODEC_ADC2_TEST
    stream_stop_capture2();
#endif

    stream_stop_playback1();

#ifdef CODEC_ADC1_TEST
    stream_close_capture1();
#endif
#ifdef CODEC_ADC2_TEST
    stream_close_capture2();
#endif

    stream_close_playback1();

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_32K);
}

enum AUD_TC_ID {
    AUD_TC_NONE,
    AUD_TC_ADDA_LOOP,
};

static uint32_t aud_tc_id = AUD_TC_NONE;

static void audio_test_adda_loop(bool on)
{
    loop_next_state = on;

    aud_tc_id = AUD_TC_ADDA_LOOP;
}

static void adda_loop_control(bool on)
{
    loop_started = on;

    if (on) {
        adda_loop_start();
    } else {
        adda_loop_stop();
    }
}

static void audio_test_exec(void)
{
    if (aud_tc_id == AUD_TC_ADDA_LOOP) {
        bool state;

        state = loop_next_state;
        if (loop_started != state) {
            adda_loop_control(state);
        }
    }
}

/////////////////////////////////// test interface /////////////////////////////
void adda_loop_app(bool on)
{
    audio_test_adda_loop(true);
}

void adda_loop_app_loop(void)
{
    audio_test_exec();

#ifndef RTOS
    extern void af_thread(void const *argument);
    af_thread(NULL);
#endif
}

int adda_loop_app_key(enum HAL_KEY_CODE_T code, enum HAL_KEY_EVENT_T event)
{
    if (code == HAL_KEY_CODE_PWR) {
        if (event == HAL_KEY_EVENT_CLICK) {
            adda_loop_app(!loop_started);
            // The key event has been processed
            return 0;
        }
    }

    // Let other applications check the key event
    return 1;
}

void adc2_test(void)
{
    TRACE(3,"%s:",__func__);
    af_open();

    adda_loop_app(true);

    while (1) {
        adda_loop_app_loop();
    }
}
#endif /* ADC2_TEST */
