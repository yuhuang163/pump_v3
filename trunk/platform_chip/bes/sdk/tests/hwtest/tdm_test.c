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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plat_addr_map.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_i2c.h"
#include "hal_i2s.h"
#include "hal_dma.h"
#include "hal_tdm.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_aud.h"
#include "tgt_hardware.h"

#if 1
#define TDM_TEST_TRACE TRACE
#else
#define TDM_TEST_TRACE(str,...)
#endif
//#define TDM_AS_I2S_TEST
//#define BITS_WIDTH 32
//#define DUMP_BUFFER

// Close I2S and VPU when test current.
// #define CLOSE_I2S_VPU

#define TDM_TEST_CHANNEL_NUM                8
#define TDM_TEST_SAMPLE_RATE                AUD_SAMPRATE_16000
#define TDM_TEST_FS_CYCLE                   HAL_TDM_FS_CYCLES_16
#define TDM_TEST_SLOT_CYCLES                HAL_TDM_SLOT_CYCLES_16


#define TDM_TEST_I2S_0_SAMPLE_RATE          TDM_TEST_SAMPLE_RATE
#define TDM_TEST_I2S_1_SAMPLE_RATE          TDM_TEST_SAMPLE_RATE
#define TDM_TEST_FRAME_SIZE                 512
#define TDM_TEST_FRAME_NUM                  4
#define TDM_TEST_PLAYBACK_CHAN              1
#define TDM_TEST_CAPTURE_CHAN               2

#define TDM_TEST_PLAYBACK_FRAME_NUM         4
#define TDM_TEST_CAPTURE_FRAME_NUM          4*(8/2)

#define TDM_TEST_RATE_TO_SIZE(n)            (((n) + (1000 - 1)) / 1000)
#define TDM_TEST_PLAYBACK_FRAME_SIZE        (TDM_TEST_RATE_TO_SIZE(TDM_TEST_I2S_0_SAMPLE_RATE) * TDM_TEST_FRAME_NUM * TDM_TEST_PLAYBACK_CHAN)
#define TDM_TEST_CAPTURE_FRAME_SIZE         (TDM_TEST_RATE_TO_SIZE(TDM_TEST_I2S_0_SAMPLE_RATE) * TDM_TEST_FRAME_NUM * TDM_TEST_CAPTURE_CHAN)
#define TDM_TEST_NON_EXP_ALIGN(val, exp)    (((val) + ((exp) - 1)) / (exp) * (exp))
#define TDM_TEST_BUFF_ALIGN                      (16)
#define TDM_TEST_SAMPLE_SIZE                2
#define TDM_TEST_PLAYBACK_SIZE                   TDM_TEST_NON_EXP_ALIGN(TDM_TEST_PLAYBACK_FRAME_SIZE * TDM_TEST_PLAYBACK_FRAME_NUM, TDM_TEST_BUFF_ALIGN)
#define TDM_TEST_CAPTURE_SIZE                    TDM_TEST_NON_EXP_ALIGN(TDM_TEST_CAPTURE_FRAME_SIZE * TDM_TEST_CAPTURE_FRAME_NUM, TDM_TEST_BUFF_ALIGN)

#define TDM_TEST_ALIGNED4                        ALIGNED(4)

POSSIBLY_UNUSED
static uint8_t TDM_TEST_ALIGNED4 tdm_test_playback_buf[TDM_TEST_PLAYBACK_SIZE];
POSSIBLY_UNUSED
static uint8_t TDM_TEST_ALIGNED4 tdm_test_capture_buf[TDM_TEST_CAPTURE_SIZE];

#if defined(DUMP_BUFFER) && defined(LIS25_TEST)
static uint8_t TDM_TEST_ALIGNED4 dump_playback_buf[TDM_TEST_PLAYBACK_SIZE];
static uint8_t TDM_TEST_ALIGNED4 dump_capture_buf[TDM_TEST_CAPTURE_SIZE];
#endif

#if defined(TDM_PLAYBACK_ONLY_TEST)
#include "audioflinger.h"
static enum AUD_BITS_T tdm_test_sample_size_to_enum(uint32_t size)
{
    if (size == 2) {
        return AUD_BITS_16;
    } else if (size == 4) {
        return AUD_BITS_24;
    } else {
        ASSERT(false, "%s: Invalid sample size: %u", __func__, size);
    }

    return 0;
}

static enum AUD_CHANNEL_NUM_T tdm_test_chan_num_to_enum(uint32_t num)
{
    if (num == 2) {
        return AUD_CHANNEL_NUM_2;
    } else if (num == 1) {
        return AUD_CHANNEL_NUM_1;
    } else {
        ASSERT(false, "%s: Invalid channel num: %u", __func__, num);
    }

    return 0;
}

static uint32_t tdm_test_loopback_data_playback(uint8_t *buf, uint32_t len)
{
    // uint32_t cur_time;
    static uint32_t cnt = 0;

  //  cur_time = hal_sys_timer_get();
    if(cnt % 300 == 0)
    {
        TDM_TEST_TRACE(1,"tx len = %d:", len/2);
        DUMP16("0x%x,", buf, len/2 < 128 ? len/2 : 128);
    }
    cnt ++;
    return 0;
}

static void tdm_test_loopback_af_start(void)
{
    TDM_TEST_TRACE(2,"%s: %d.", __func__, __LINE__ );

    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;
    uint32_t i;

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_52M);

    uint8_t *buf;
    buf = (uint8_t *)&tdm_test_playback_buf[0];
    for (i = 0; i < sizeof(tdm_test_playback_buf); i++) {
        buf[i] = (uint8_t)i;
    }
    TDM_TEST_TRACE(2,"%s: %d.", __func__, __LINE__ );

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits = AUD_BITS_16;//tdm_test_sample_size_to_enum(TDM_TEST_SAMPLE_SIZE);
    stream_cfg.channel_num = TDM_TEST_CHANNEL_NUM;
    stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    stream_cfg.sample_rate = TDM_TEST_I2S_0_SAMPLE_RATE;
    stream_cfg.device = AUD_STREAM_USE_TDM0_MASTER;
    stream_cfg.handler = tdm_test_loopback_data_playback;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = tdm_test_playback_buf;
    stream_cfg.data_size = sizeof(tdm_test_playback_buf);
    stream_cfg.fs_cycles = TDM_TEST_FS_CYCLE;
    stream_cfg.slot_cycles = TDM_TEST_SLOT_CYCLES;

    TDM_TEST_TRACE(3,"[%s] 1 playback sample_rate: %d, bits = %d", __func__, stream_cfg.sample_rate, stream_cfg.bits);

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);


    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);

}

static void tdm_test_loopback_af_stop(void)
{
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_32K);
}

void tdm_test(void)
{
    uint32_t DUMMY;
    DUMMY = (uint32_t)tdm_test_loopback_af_stop;
    DUMMY = DUMMY;
    DUMMY = (uint32_t)tdm_test_capture_buf;
    DUMMY = (uint32_t)tdm_test_sample_size_to_enum;
    DUMMY = (uint32_t)tdm_test_chan_num_to_enum;
    hal_iomux_set_i2s0_mclk();//hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_MCU_SPDIF0);
    af_open();
    tdm_test_loopback_af_start();
    while(1) {hal_sys_timer_delay(MS_TO_TICKS(100));};

}
#elif defined(TDM_LOOPBACK_TEST)
#include "audioflinger.h"

static enum AUD_BITS_T tdm_test_sample_size_to_enum(uint32_t size)
{
    if (size == 2) {
        return AUD_BITS_16;
    } else if (size == 4) {
        return AUD_BITS_24;
    } else {
        ASSERT(false, "%s: Invalid sample size: %u", __func__, size);
    }

    return 0;
}

static enum AUD_CHANNEL_NUM_T tdm_test_chan_num_to_enum(uint32_t num)
{
    if (num == 2) {
        return AUD_CHANNEL_NUM_2;
    } else if (num == 1) {
        return AUD_CHANNEL_NUM_1;
    } else {
        ASSERT(false, "%s: Invalid channel num: %u", __func__, num);
    }

    return 0;
}

static uint32_t tdm_test_loopback_data_playback(uint8_t *buf, uint32_t len)
{
    // uint32_t cur_time;
    static uint32_t cnt = 0;

  //  cur_time = hal_sys_timer_get();
    if(cnt % 300 == 0)
    {
        TDM_TEST_TRACE(1,"tx len = %d:", len/2);
        DUMP16("0x%x,", buf, len/2 < 128 ? len/2 : 128);
    }
    cnt ++;
    return 0;
}

static uint32_t tdm_test_loopback_data_capture(uint8_t *buf, uint32_t len)
{
   // uint32_t cur_time;
    static uint32_t cnt = 0;

  //  cur_time = hal_sys_timer_get();
    if(cnt % 300 == 0)
    {
        TDM_TEST_TRACE(1,"rx len = %d:", len/2);
        DUMP16("0x%x,", buf, len/2 < 128 ? len/2 : 128);
    }
    cnt ++;
    return 0;
}

static void tdm_test_loopback_af_start(void)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_52M);
    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.channel_num = TDM_TEST_CHANNEL_NUM;
    stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    stream_cfg.sample_rate = TDM_TEST_I2S_0_SAMPLE_RATE;
    stream_cfg.device = AUD_STREAM_USE_TDM1_MASTER; // AUD_STREAM_USE_INT_CODEC;
    stream_cfg.handler = tdm_test_loopback_data_playback;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = tdm_test_capture_buf; // tdm_test_playback_buf;
    stream_cfg.data_size = sizeof(tdm_test_capture_buf); // sizeof(tdm_test_playback_buf);
    stream_cfg.fs_cycles = TDM_TEST_FS_CYCLE;
    stream_cfg.slot_cycles = TDM_TEST_SLOT_CYCLES;

    TDM_TEST_TRACE(3,"[%s] 1 playback sample_rate: %d, bits = %d", __func__, stream_cfg.sample_rate, stream_cfg.bits);

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits = AUD_BITS_16;//tdm_test_sample_size_to_enum(TDM_TEST_SAMPLE_SIZE);
    stream_cfg.channel_num = TDM_TEST_CHANNEL_NUM;
    stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    stream_cfg.sample_rate = TDM_TEST_I2S_0_SAMPLE_RATE;
    stream_cfg.device = AUD_STREAM_USE_TDM0_SLAVE;
    stream_cfg.handler = tdm_test_loopback_data_capture;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = tdm_test_capture_buf;
    stream_cfg.data_size = sizeof(tdm_test_capture_buf);
    stream_cfg.fs_cycles = TDM_TEST_FS_CYCLE;
    stream_cfg.slot_cycles = TDM_TEST_SLOT_CYCLES;

    TDM_TEST_TRACE(3,"[%s] 2 capture sample_rate: %d, bits = %d", __func__, stream_cfg.sample_rate, stream_cfg.bits);

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open catpure failed: %d", ret);

    ret = af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "af_stream_setup catpure failed: %d", ret);
    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);

    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "af_stream_start catpure failed: %d", ret);
}

static void tdm_test_loopback_af_stop(void)
{
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_32K);
}

void tdm_test(void)
{
    uint32_t DUMMY;
    DUMMY = (uint32_t)tdm_test_loopback_af_stop;
    DUMMY = DUMMY;
    DUMMY = (uint32_t)tdm_test_playback_buf;
  //  DUMMY = (uint32_t)tdm_test_capture_buf;
  //  DUMMY = (uint32_t)tdm_test_loopback_data_capture;
    DUMMY = (uint32_t)tdm_test_sample_size_to_enum;
    DUMMY = (uint32_t)tdm_test_chan_num_to_enum;
    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_MCU_SPDIF0);
    af_open();
    tdm_test_loopback_af_start();
    while(1) {hal_sys_timer_delay(MS_TO_TICKS(100));};

}

#elif defined(LIS25_TEST)
#include "lis25ba.h"
#include "audioflinger.h"
static uint32_t tdm_test_cap_rpos;
static uint32_t tdm_test_cap_wpos;

// static uint32_t I2S_TDM_BUF_ALIGN i2s_tdm_rx_buf[I2S_TDM_FRAME_NUM][I2S_TDM_FRAME_SIZE/4];
// static uint32_t rf_frame_num = I2S_TDM_FRAME_NUM;

struct LIST25_CONFIG_T lis25_cfg =
{
    true,
    true,
    true,
    true,
    true,
    LIS25_WCLK_16K,
    LIS25_MAPPING_012,
    LIS25_DATA_VALID_RISE,
    LIS25_DELAYED_DISABLE,
};

static enum AUD_BITS_T tdm_test_sample_size_to_enum(uint32_t size)
{
    if (size == 2) {
        return AUD_BITS_16;
    } else if (size == 4) {
        return AUD_BITS_24;
    } else {
        ASSERT(false, "%s: Invalid sample size: %u", __func__, size);
    }

    return 0;
}

static enum AUD_CHANNEL_NUM_T tdm_test_chan_num_to_enum(uint32_t num)
{
    if (num == 2) {
        return AUD_CHANNEL_NUM_2;
    } else if (num == 1) {
        return AUD_CHANNEL_NUM_1;
    } else {
        ASSERT(false, "%s: Invalid channel num: %u", __func__, num);
    }

    return 0;
}

static uint32_t tdm_test_data_playback(uint8_t *buf, uint32_t len)
{
    int16_t *src;
    int16_t *dst;
    int16_t *src_end;
    int16_t *dst_end;
    uint32_t avail;
    uint32_t slot_index = 0;
    static uint32_t cnt = 0;

    if (tdm_test_cap_wpos >= tdm_test_cap_rpos)
    {
        avail = tdm_test_cap_wpos - tdm_test_cap_rpos;
    }
    else
    {
        avail = sizeof(tdm_test_capture_buf) + tdm_test_cap_wpos - tdm_test_cap_rpos;
    }

    if ((avail/8) * 2 * TDM_TEST_PLAYBACK_CHAN / TDM_TEST_CAPTURE_CHAN >= len)
    {
        src = (int16_t *)(tdm_test_capture_buf + tdm_test_cap_rpos);
        src_end = (int16_t *)(tdm_test_capture_buf + sizeof(tdm_test_capture_buf));
        dst = (int16_t *)buf;
        dst_end = (int16_t *)(buf + len);

        while (dst < dst_end) {
            if(slot_index == 1 /*|| slot_index == 1*/)
            {
                *dst = (*src)>>1;
                dst++;
            }
            src++;
            slot_index = slot_index + 1 == 8 ? 0: slot_index + 1;
            if (src == src_end) {
                src = (int16_t *)tdm_test_capture_buf;
            }
#if (TDM_TEST_PLAYBACK_CHAN == 2) && (TDM_TEST_CAPTURE_CHAN == 1)
            *dst = *(dst - 1);
            dst++;
#endif
        }
        while(slot_index++ < 8)
        {
            src++;
            if (src == src_end) {
                src = (int16_t *)tdm_test_capture_buf;
            }
        }
        tdm_test_cap_rpos = (uint32_t)src - (uint32_t)tdm_test_capture_buf;
#ifdef DUMP_BUFFER
        memcpy(dump_playback_buf,buf,len);
#endif
    }
    else {
        memset(buf, 0, len);
    }

    if(cnt % 300 == 0)
    {
         uint32_t cur_time = hal_sys_timer_get();
         TDM_TEST_TRACE(5,"[0x%x] playback: len=%d wpos=%d rpos=%d avail=% d.", cur_time, len, tdm_test_cap_wpos, tdm_test_cap_rpos, avail);
        // DUMP16("0x%04x,", buf, len < 64 ? len : 64);
    }
    cnt ++;

    return 0;
}

static uint32_t tdm_test_data_capture(uint8_t *buf, uint32_t len)
{
    uint32_t cur_time;
    static uint32_t cnt = 0;
    cur_time = hal_sys_timer_get();

    tdm_test_cap_wpos += len;
    if (tdm_test_cap_wpos >= sizeof(tdm_test_capture_buf)) {
        tdm_test_cap_wpos = 0;
    }

#ifdef DUMP_BUFFER
    memcpy(dump_capture_buf,buf,len);
#endif
    if(cnt % 300 == 0)
    {
        cur_time = cur_time;
        TDM_TEST_TRACE(4,"[0x%x] capture : len= %d wpos = %d rpos = %d.",
                     cur_time, len, tdm_test_cap_wpos, tdm_test_cap_rpos);

        //DUMP16("0x%04x,", buf, splicing_len < 32 ? splicing_len : 32);

    }
    cnt ++;

    return 0;
}

static void tdm_test_af_start(void)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.sync_start = true;
    stream_cfg.bits = tdm_test_sample_size_to_enum(2);
    stream_cfg.channel_num = tdm_test_chan_num_to_enum(TDM_TEST_PLAYBACK_CHAN);
    stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 /*| AUD_CHANNEL_MAP_CH1*/;
    stream_cfg.sample_rate = TDM_TEST_I2S_0_SAMPLE_RATE;
    stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.vol = TGT_VOLUME_LEVEL_MAX;
    stream_cfg.handler = tdm_test_data_playback;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = tdm_test_playback_buf;
    stream_cfg.data_size = sizeof(tdm_test_playback_buf);

    TDM_TEST_TRACE(4,"[%s] 1 playback sample_rate: %d, bits = %d,data_size = %d.",
            __func__, stream_cfg.sample_rate, stream_cfg.bits,stream_cfg.data_size);

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.sync_start = false;
    stream_cfg.align      = AUD_DATA_ALIGN_LEFT_JUSTIFIED;
    stream_cfg.fs_edge = AUD_FS_FIRST_EDGE_POS;
    stream_cfg.bits = AUD_BITS_16;//tdm_test_sample_size_to_enum(TDM_TEST_SAMPLE_SIZE);
    stream_cfg.channel_num = 8;
    stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    stream_cfg.sample_rate = TDM_TEST_I2S_0_SAMPLE_RATE;
    stream_cfg.device = AUD_STREAM_USE_TDM0_MASTER;
    stream_cfg.handler = tdm_test_data_capture;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = tdm_test_capture_buf;
    stream_cfg.data_size = sizeof(tdm_test_capture_buf);
    stream_cfg.fs_cycles = HAL_TDM_FS_CYCLES_16;
    stream_cfg.slot_cycles = 16;
    TDM_TEST_TRACE(4,"[%s] 2 capture sample_rate: %d, bits = %d,data_size = %d.",
            __func__, stream_cfg.sample_rate, stream_cfg.bits,stream_cfg.data_size);

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open catpure failed: %d", ret);
    //ret = af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
    //ASSERT(ret == 0, "af_stream_setup catpure failed: %d", ret);
    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);

    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "af_stream_start catpure failed: %d", ret);
}

static void tdm_test_af_stop(void)
{
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_32K);
}

void set_mclk(void)
{
    volatile uint32_t *p1 = (uint32_t*)0x4008001c;
    volatile uint32_t *p2 = (uint32_t*)0x4000005c;
    volatile uint32_t *p3 = (uint32_t*)0x40080068;
    uint32_t tmp;

    tmp = *p1;
    *p1 = tmp | 0x7 ;

    tmp = (*p2 & (~0x3c000));
    *p2 = tmp | (0x7 << 14) ;

    tmp = (*p3 & (~0x3fff));
    *p3 = (tmp | (1<<13) | 0x0e);
}

void tdm_test(void)
{
    uint32_t DUMMY;
    DUMMY = (uint32_t)tdm_test_af_stop;
    DUMMY = DUMMY;
    int ret;

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_208M);
    TRACE(1, "########CPU freq: %u#######", hal_sys_timer_calc_cpu_freq(5, 0));

    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_MCU_SPDIF0);

    ret = lis25ba_open();
    if(ret)
    {
       TDM_TEST_TRACE(2,"%s: lis25ba_init failed! ret = %d.",__func__,ret);
       goto _func_end;
    }
    else
    {
       TDM_TEST_TRACE(1,"%s: lis25ba_init ok.",__func__);
    }

    ret = lis25ba_setup(&lis25_cfg);
    if(ret)
    {
       TDM_TEST_TRACE(2,"%s: lis25ba_setup failed! ret = %d.",__func__,ret);
       goto _func_end;
    }
    else
    {
       TDM_TEST_TRACE(1,"%s: lis25ba_setup ok.",__func__);
    }

    ret = lis25ba_start();
    if(ret)
    {
       TDM_TEST_TRACE(2,"%s: lis25ba_start failed! ret = %d.",__func__,ret);
       goto _func_end;
    }
    else
    {
       TDM_TEST_TRACE(1,"%s: lis25ba_start ok.",__func__);
    }

    af_open();
    tdm_test_af_start();
#ifdef CLOSE_I2S_VPU  // CLOSE I2S,MCLK, VPU
    ret = af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "af_stream_stop catpure failed: %d", ret);
    lis25ba_stop();
    hal_cmu_clock_out_disable();
#endif
    while(1) {
#if defined(DUMP_BUFFER)
        uint32_t i;
        uint16_t* p;
        TRACE(0,"dump_capture_buf:");
        for(i = 0; i < 128/32; i++)
        {
            p = (uint16_t*)dump_capture_buf + i*32;
            DUMP16("0x%04x,", p ,32);
        }
        TRACE(0,"dump_playback_buf:");
        DUMP16("0x%04x,", dump_playback_buf,32);
        for(i = 0; i < 128/32; i++)
        {
            p = (uint16_t*)dump_playback_buf + i*32;
            DUMP16("0x%04x,", p ,32);
        }
 #endif
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    };
_func_end:
    return;

}

#else // TDM stream Test.

static uint32_t TDM_BUF_ALIGN tdm0_rx_buf[TDM_TEST_FRAME_NUM][TDM_TEST_FRAME_SIZE/4];
#ifndef NO_NATIVE_TDM
static uint32_t TDM_BUF_ALIGN tdm0_tx_buf[TDM_TEST_FRAME_NUM][TDM_TEST_FRAME_SIZE/4];
#endif

static struct HAL_DMA_DESC_T tdm0_rx_dma_desc[TDM_TEST_FRAME_NUM];
#ifndef NO_NATIVE_TDM
static struct HAL_DMA_DESC_T tdm0_tx_dma_desc[TDM_TEST_FRAME_NUM];
#endif
static void tdm0_rx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % (TDM_TEST_I2S_0_SAMPLE_RATE/TDM_TEST_FRAME_NUM) == 0) {
        TDM_TEST_TRACE(5,"TDM-0-RX: remains=%d, error=%d, cnt=%d, dst = 0x%x,ctrl = 0x%x.",
            remains, error, cnt,lli->dst,lli->ctrl);
       // DUMP8("0x%x,",(uint8_t*)lli->src,32);
        //DUMP8("0x%x,",(uint8_t*)lli->dst,32);
        //DUMP8("0x%x,",(uint8_t*)lli->dst + 32 ,32);
    }
}

#ifndef NO_NATIVE_TDM
static void tdm0_tx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % (TDM_TEST_I2S_0_SAMPLE_RATE/TDM_TEST_FRAME_NUM) == 0) {
        TDM_TEST_TRACE(5,"TDM-0-TX: remains=%d, error=%d, cnt=%d, src = 0x%x, ctrl = 0x%x.",
            remains, error, cnt,lli->src,lli->ctrl);
        //DUMP8("0x%x,",(uint8_t*)lli->src,32);
        //DUMP8("0x%x,",(uint8_t*)lli->src + 32,32);
       // DUMP8("0x%x,",(uint8_t*)lli->dst,32);
    }
}
#endif

#ifdef TDM_AS_I2S_TEST
void tdm_0_as_i2s_test(void)
{
    int i;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    uint32_t bits;
    int ret;

    TDM_TEST_TRACE(1,"%s: Start.", __func__);
#if BITS_WIDTH == 16
    bits = 16;
#elif BITS_WIDTH == 32
    bits = 32;
#else
    // default bits is 32
    bits = 32;
#endif

#ifndef NO_NATIVE_TDM
    uint8_t *buf;
    buf = (uint8_t *)&tdm0_tx_buf[0];

    if(bits == 16) {
        for (i = 0; i < sizeof(tdm0_tx_buf); i++) {
            if(i%4 == 1)
            {
                buf[i] = (uint8_t)0xff;
            }
            else if(i%4 == 3)
            {
                buf[i] = (uint8_t)0xaa;
            }
            else
            {
                buf[i] = (uint8_t)0x00;
            }
        }
    }
    else {
        for (i = 0; i < sizeof(tdm0_tx_buf); i++) {
            if(i%8 == 3)
            {
                buf[i] = (uint8_t)0xff;
            }
            else if(i%8 == 7)
            {
                buf[i] = (uint8_t)0xaa;
            }
            else
            {
                buf[i] = (uint8_t)0x00;
            }

        }
    }

    for (i = 0; i < sizeof(tdm0_tx_buf)/32; i++)
    {
        DUMP8("0x%x,",(uint8_t*)&buf[i*32], 32);
    }
    ret = hal_tdm_open(HAL_I2S_ID_0, AUD_STREAM_PLAYBACK, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open tdm tx master");
#endif

    ret = hal_tdm_open(HAL_I2S_ID_0, AUD_STREAM_CAPTURE, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open tdm rx master");


#ifndef NO_NATIVE_TDM
    ret = hal_tdm_as_i2s_setup_stream(HAL_I2S_ID_0, AUD_STREAM_PLAYBACK, (uint32_t)TDM_TEST_I2S_0_SAMPLE_RATE, bits);
    ASSERT(ret == 0, "Failed to setup tdm tx stream");
#endif
    ret = hal_tdm_as_i2s_setup_stream(HAL_I2S_ID_0, AUD_STREAM_CAPTURE, (uint32_t)TDM_TEST_I2S_0_SAMPLE_RATE, bits);
    ASSERT(ret == 0, "Failed to setup tdm rx stream");

#ifndef NO_NATIVE_TDM
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; //useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = HAL_AUDMA_I2S0_TX;
    dma_cfg.dst_width = bits == 16 ? HAL_DMA_WIDTH_HALFWORD : HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = tdm0_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.src_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.src_tsize = bits == 16 ? sizeof(tdm0_tx_buf[0]) / 2 : sizeof(tdm0_tx_buf[0]) / 4;
    dma_cfg.src_width = bits == 16 ? HAL_DMA_WIDTH_HALFWORD : HAL_DMA_WIDTH_WORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < TDM_TEST_FRAME_NUM; i++) {
        dma_cfg.src = (uint32_t)&tdm0_tx_buf[i][0];
        ret = hal_audma_init_desc(&tdm0_tx_dma_desc[i], &dma_cfg, &tdm0_tx_dma_desc[(i + 1) % TDM_TEST_FRAME_NUM], (i + 1) % (TDM_TEST_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for tx");
    }

    ret = hal_audma_sg_start(&tdm0_tx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for tx");
#endif

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = bits == 16 ? HAL_DMA_WIDTH_HALFWORD : HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = tdm0_rx_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_I2S0_RX;
    dma_cfg.src_tsize = bits == 16 ? sizeof(tdm0_tx_buf[0]) / 2 : sizeof(tdm0_tx_buf[0]) / 4;
    dma_cfg.src_width = bits == 16 ? HAL_DMA_WIDTH_HALFWORD : HAL_DMA_WIDTH_WORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < TDM_TEST_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&tdm0_rx_buf[i][0];
        ret = hal_audma_init_desc(&tdm0_rx_dma_desc[i], &dma_cfg, &tdm0_rx_dma_desc[(i + 1) % TDM_TEST_FRAME_NUM], (i + 1) % (TDM_TEST_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for rx");
    }

    ret = hal_audma_sg_start(&tdm0_rx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for rx");
#ifndef NO_NATIVE_TDM
    ret = hal_tdm_start_stream(HAL_I2S_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "Failed to start tdm tx stream");
#endif
    ret = hal_tdm_start_stream(HAL_I2S_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "Failed to start tdm rx stream");

    TDM_TEST_TRACE(1,"%s: done.", __func__);
}

#else // TDM_AS_I2S_TEST

void tdm_0_test(void)
{
    int i;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    struct HAL_TDM_CONFIG_T tdm_cfg;
    int ret;

    TDM_TEST_TRACE(1,"%s: Start.", __func__);
#ifndef NO_NATIVE_TDM
    uint8_t *buf;
    buf = (uint8_t *)&tdm0_tx_buf[0];
    for (i = 0; i < TDM_TEST_FRAME_NUM * TDM_TEST_FRAME_SIZE; i++) {
        buf[i] = (uint8_t)i;
    }

    ret = hal_tdm_open(HAL_I2S_ID_0, AUD_STREAM_PLAYBACK, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open tdm tx master");
#endif

    ret = hal_tdm_open(HAL_I2S_ID_0, AUD_STREAM_CAPTURE, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open tdm rx master");

    memset(&tdm_cfg, 0, sizeof(tdm_cfg));
    tdm_cfg.mode = HAL_TDM_MODE_FS_ASSERTED_AT_FIRST;
    tdm_cfg.edge = HAL_TDM_FS_EDGE_POSEDGE;
    tdm_cfg.cycles = HAL_TDM_CYCLES_256;
    tdm_cfg.fs_cycles = HAL_TDM_FS_CYCLES_8;
    tdm_cfg.slot_cycles = HAL_TDM_SLOT_CYCLES_32;
    tdm_cfg.data_offset = 0;
    tdm_cfg.bits = 32;
    tdm_cfg.channel_num = 4;

#ifndef NO_NATIVE_TDM
    ret = hal_tdm_setup_stream(HAL_I2S_ID_0, AUD_STREAM_PLAYBACK, (uint32_t)TDM_TEST_I2S_0_SAMPLE_RATE, &tdm_cfg);
    ASSERT(ret == 0, "Failed to setup tdm tx stream");
#endif
    ret = hal_tdm_setup_stream(HAL_I2S_ID_0, AUD_STREAM_CAPTURE, (uint32_t)TDM_TEST_I2S_0_SAMPLE_RATE, &tdm_cfg);
    ASSERT(ret == 0, "Failed to setup tdm rx stream");

#ifndef NO_NATIVE_TDM
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; //useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = HAL_AUDMA_I2S0_TX;
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = tdm0_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.src_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.src_tsize = sizeof(tdm0_tx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < TDM_TEST_FRAME_NUM; i++) {
        dma_cfg.src = (uint32_t)&tdm0_tx_buf[i][0];
        ret = hal_audma_init_desc(&tdm0_tx_dma_desc[i], &dma_cfg, &tdm0_tx_dma_desc[(i + 1) % TDM_TEST_FRAME_NUM], (i + 1) % (TDM_TEST_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for tx");
    }

    ret = hal_audma_sg_start(&tdm0_tx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for tx");
#endif

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = tdm0_rx_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_I2S0_RX;
    dma_cfg.src_tsize = sizeof(tdm0_rx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < TDM_TEST_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&tdm0_rx_buf[i][0];
        ret = hal_audma_init_desc(&tdm0_rx_dma_desc[i], &dma_cfg, &tdm0_rx_dma_desc[(i + 1) % TDM_TEST_FRAME_NUM], (i + 1) % (TDM_TEST_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for rx");
    }

    ret = hal_audma_sg_start(&tdm0_rx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for rx");
#ifndef NO_NATIVE_TDM
    ret = hal_tdm_start_stream(HAL_I2S_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "Failed to start tdm tx stream");
#endif
    ret = hal_tdm_start_stream(HAL_I2S_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "Failed to start tdm rx stream");

    TDM_TEST_TRACE(1,"%s: done.", __func__);
}

#endif // TDM_AS_I2S_TEST

#ifdef I2S1_BASE
static uint32_t TDM_BUF_ALIGN tdm1_rx_buf[TDM_TEST_FRAME_NUM][TDM_TEST_FRAME_SIZE/4];
#ifndef NO_NATIVE_TDM
static uint32_t TDM_BUF_ALIGN tdm1_tx_buf[TDM_TEST_FRAME_NUM][TDM_TEST_FRAME_SIZE/4];
#endif
static struct HAL_DMA_DESC_T tdm1_rx_dma_desc[TDM_TEST_FRAME_NUM];
#ifndef NO_NATIVE_TDM
static struct HAL_DMA_DESC_T tdm1_tx_dma_desc[TDM_TEST_FRAME_NUM];
#endif
static void tdm1_rx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % (TDM_TEST_I2S_1_SAMPLE_RATE/TDM_TEST_FRAME_NUM) == 0) {
        TDM_TEST_TRACE(5,"TDM-1-RX: remains=%d, error=%d, cnt=%d, dst = 0x%x,ctrl = 0x%x.",
            remains, error, cnt,lli->dst,lli->ctrl);
       // DUMP8("0x%x,",(uint8_t*)lli->src,32);
        DUMP8("0x%x,",(uint8_t*)lli->dst,32);
        DUMP8("0x%x,",(uint8_t*)lli->dst + 32 ,32);
    }
}
#ifndef NO_NATIVE_TDM
static void tdm1_tx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % (TDM_TEST_I2S_1_SAMPLE_RATE/TDM_TEST_FRAME_NUM) == 0) {
        TDM_TEST_TRACE(5,"TDM-1-TX: remains=%d, error=%d, cnt=%d, src = 0x%x,ctrl = 0x%x.",
            remains, error, cnt,lli->src,lli->ctrl);
        DUMP8("0x%x,",(uint8_t*)lli->src,32);
        DUMP8("0x%x,",(uint8_t*)lli->src + 32,32);
       // DUMP8("0x%x,",(uint8_t*)lli->dst,32);
    }
}
#endif
void tdm_1_test(void)
{
    int i;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    struct HAL_TDM_CONFIG_T tdm_cfg;
    int ret;

    TDM_TEST_TRACE(1,"%s: Start.", __func__);
#ifndef NO_NATIVE_TDM
    uint8_t *buf;
    buf = (uint8_t *)&tdm1_tx_buf[0];
    for (i = 0; i < TDM_TEST_FRAME_NUM * TDM_TEST_FRAME_SIZE; i++) {
        buf[i] = (uint8_t)i;
    }
    ret = hal_tdm_open(HAL_I2S_ID_1, AUD_STREAM_PLAYBACK, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open tdm tx master");
#endif

    ret = hal_tdm_open(HAL_I2S_ID_1, AUD_STREAM_CAPTURE, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open tdm rx master");

    memset(&tdm_cfg, 0, sizeof(tdm_cfg));
    tdm_cfg.mode = HAL_TDM_MODE_FS_ASSERTED_AT_FIRST;
    tdm_cfg.edge = HAL_TDM_FS_EDGE_POSEDGE;
    tdm_cfg.cycles = HAL_TDM_CYCLES_128;
    tdm_cfg.fs_cycles = HAL_TDM_FS_CYCLES_16;
    tdm_cfg.slot_cycles = HAL_TDM_SLOT_CYCLES_16;
    tdm_cfg.bits = 32;
    tdm_cfg.channel_num = 4;
    tdm_cfg.data_offset = 0;

#ifndef NO_NATIVE_TDM
    TDM_TEST_TRACE(1,"%s: call hal_tdm_setup_stream .", __func__);
    ret = hal_tdm_setup_stream(HAL_I2S_ID_1, AUD_STREAM_PLAYBACK, (uint32_t)TDM_TEST_I2S_1_SAMPLE_RATE, &tdm_cfg);
    ASSERT(ret == 0, "Failed to setup tdm tx stream");
#endif
    ret = hal_tdm_setup_stream(HAL_I2S_ID_1, AUD_STREAM_CAPTURE, (uint32_t)TDM_TEST_I2S_1_SAMPLE_RATE, &tdm_cfg);
    ASSERT(ret == 0, "Failed to setup tdm rx stream");

#ifndef NO_NATIVE_TDM
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; //useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = HAL_AUDMA_I2S1_TX;
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = tdm1_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.src_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.src_tsize = sizeof(tdm1_tx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < TDM_TEST_FRAME_NUM; i++) {
        dma_cfg.src = (uint32_t)&tdm1_tx_buf[i][0];
        ret = hal_audma_init_desc(&tdm1_tx_dma_desc[i], &dma_cfg, &tdm1_tx_dma_desc[(i + 1) % TDM_TEST_FRAME_NUM], (i + 1) % (TDM_TEST_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for tx");
    }

    ret = hal_audma_sg_start(&tdm1_tx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for tx");
#endif

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = tdm1_rx_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_I2S1_RX;
    dma_cfg.src_tsize = sizeof(tdm1_rx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < TDM_TEST_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&tdm1_rx_buf[i][0];
        ret = hal_audma_init_desc(&tdm1_rx_dma_desc[i], &dma_cfg, &tdm1_rx_dma_desc[(i + 1) % TDM_TEST_FRAME_NUM], (i + 1) % (TDM_TEST_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for rx");
    }

    ret = hal_audma_sg_start(&tdm1_rx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for rx");
#ifndef NO_NATIVE_TDM
    ret = hal_tdm_start_stream(HAL_I2S_ID_1, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "Failed to start tdm tx stream");
#endif
    ret = hal_tdm_start_stream(HAL_I2S_ID_1, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "Failed to start tdm rx stream");

    TDM_TEST_TRACE(1,"%s: done", __func__);
}
#endif

void tdm_test()
{
    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);
#ifdef TDM_AS_I2S_TEST
    tdm_0_as_i2s_test();
#else
    tdm_0_test();
#endif
#ifdef I2S1_BASE
    tdm_1_test();
#endif
}
#endif

