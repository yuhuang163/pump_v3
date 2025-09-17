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
#ifdef CHIP_FUNCTION_TEST_I2S
#include "audioflinger.h"
#include <string.h>
#include "hal_trace.h"
#include "hal_location.h"
#include "cqueue.h"
#include "hal_sysfreq.h"
#ifdef RTOS
#include "cmsis_os.h"
#else
#include "hal_timer.h"
#endif
#include "hal_cmu.h"
#ifdef AD_I2S_LOOP_TEST
#include "i2s_audio.h"
#endif

#define I2S_LOOP_SAMPLE_RATE AUD_SAMPRATE_48000
#define I2S0_AUD_STREAM_ID AUD_STREAM_ID_1
#define I2S1_AUD_STREAM_ID AUD_STREAM_ID_2

#define MIDDLE_BUFFER_SIZE (1536*2)
#define I2S_BUFFER_SIZE (MIDDLE_BUFFER_SIZE * 2)
#define DAC_BUFFER_SIZE (MIDDLE_BUFFER_SIZE * 2)

static SRAM_BSS_LOC uint8_t i2s0_capture_buffer[I2S_BUFFER_SIZE];
static SRAM_BSS_LOC uint8_t i2s0_playback_buffer[I2S_BUFFER_SIZE];

static uint8_t i2s0_cb_input_count = 0;
static uint8_t i2s0_cb_output_count = 0;

static uint8_t i2s0_log = 0;

static SRAM_BSS_LOC uint8_t i2s1_capture_buffer[I2S_BUFFER_SIZE];
static SRAM_BSS_LOC uint8_t i2s1_playback_buffer[I2S_BUFFER_SIZE];

static uint8_t i2s1_cb_input_count = 0;
static uint8_t i2s1_cb_output_count = 0;

//static uint8_t i2s1_cb_count = 0;
static uint8_t i2s1_log = 0;

CQueue i2s0_loop_output_queue;
#define I2S_LOOP_OUTPUT_QUEUE_BUFFER_SIZE (MIDDLE_BUFFER_SIZE * 4)
static SRAM_BSS_LOC uint8_t i2s0_loop_output_queue_buf[I2S_LOOP_OUTPUT_QUEUE_BUFFER_SIZE];


CQueue i2s1_loop_output_queue;
static SRAM_BSS_LOC uint8_t i2s1_loop_output_queue_buf[I2S_LOOP_OUTPUT_QUEUE_BUFFER_SIZE];


static uint32_t i2s0_input_cb(uint8_t *buf, uint32_t len)
{
    uint32_t queue_len = 0;
    uint32_t old_queue_len = 0;
    static uint32_t index = 0;

    i2s0_cb_input_count++;
    i2s0_log = 0;
    if (i2s0_cb_input_count == 100)
    {
        i2s0_log = 1;
        i2s0_cb_input_count = 0;
        //TRACE(0,"[adc_cb] len = %d", len);
    }

    old_queue_len = LengthOfCQueue(&i2s0_loop_output_queue);
    if (CQ_OK != EnCQueue(&i2s0_loop_output_queue, buf, len))
    {
        TRACE(0,"adc en queue error.");
    }
    queue_len = LengthOfCQueue(&i2s0_loop_output_queue);
    //if (i2s0_log)
    {
        TRACE(0,"[INPUT] old_len = %d, queue_len = %d, len = %d, index = 0x%x",
            old_queue_len, queue_len, len, index++);
    }

    return len;
}

static uint32_t i2s0_output_cb(uint8_t *buf, uint32_t len)
{
    uint32_t queue_len = 0;
    uint32_t old_queue_len = 0;
    static uint32_t index = 0;

    i2s0_cb_output_count++;
    i2s0_log = 0;
    if (i2s0_cb_output_count == 100)
    {
        i2s0_log = 1;
        i2s0_cb_output_count = 0;
    }
    memset(buf, 0, len);

    old_queue_len = LengthOfCQueue(&i2s0_loop_output_queue);
    if (CQ_OK != PullCQueue(&i2s0_loop_output_queue, buf, len))
    {
        TRACE(0,"dac PULL queue error.");
    }

    queue_len = LengthOfCQueue(&i2s0_loop_output_queue);

    /*
    if (CQ_OK != DeCQueue(&i2s0_loop_output_queue, NULL, len))
    {
        TRACE(0,"dac de queue error.");
    }
    queue_len = LengthOfCQueue(&i2s0_loop_output_queue);
    */
    //if (i2s0_log)
    {
        TRACE(0,"[OUTPUT] old_len = %d queue_len = %d len = %d, index = 0x%x",
            old_queue_len, queue_len, len, index++);
    }

    return len;
}

static uint32_t i2s1_input_cb(uint8_t *buf, uint32_t len)
{
    uint32_t queue_len = 0;
    i2s1_cb_input_count++;
    i2s1_log = 0;
    if (i2s1_cb_input_count == 100)
    {
        i2s1_log = 1;
        i2s1_cb_input_count = 0;
        //TRACE(0,"[adc_cb] len = %d", len);
    }

    if (CQ_OK != EnCQueue(&i2s1_loop_output_queue, buf, len))
    {
        //TRACE(0,"adc en queue error.");
    }
    queue_len = LengthOfCQueue(&i2s1_loop_output_queue);
    //if (i2s1_log)
    {
        TRACE(0,"[i2s0_input_cb] queue_len = %d", queue_len);
    }

    return len;
}

static uint32_t i2s1_output_cb(uint8_t *buf, uint32_t len)
{
    uint32_t queue_len = 0;
    i2s1_cb_output_count++;
    i2s1_log = 0;
    if (i2s1_cb_output_count == 100)
    {
        i2s1_log = 1;
        i2s1_cb_output_count = 0;
    }
    memset(buf, 0, len);


    if (CQ_OK != PeekCQueueToBuf(&i2s1_loop_output_queue, buf, len))
    {
        //TRACE(0,"dac peek queue error.");
    }
    if (CQ_OK != DeCQueue(&i2s1_loop_output_queue, NULL, len))
    {
        //TRACE(0,"dac de queue error.");
    }
    queue_len = LengthOfCQueue(&i2s1_loop_output_queue);
    if (i2s1_log)
    {
        TRACE(0,"[i2s0_output_cb] queue_len = %d", queue_len);
    }

    return len;
}

extern bool start_print_log;
static void _i2s_loopback_test(uint8_t mode, enum AUD_STREAM_USE_DEVICE_T in_dev, enum AUD_STREAM_USE_DEVICE_T out_dev)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    uint32_t ret;

    TRACE(0,"%s, sample_rate = %d", __func__, I2S_LOOP_SAMPLE_RATE);

    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);
    //hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_SYS);
    //ret = hal_iomux_init(pinmux_i2s, sizeof(pinmux_i2s)/sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
    //ASSERT(ret == 0, "Failed to init iomux");

    InitCQueue(&i2s0_loop_output_queue, I2S_LOOP_OUTPUT_QUEUE_BUFFER_SIZE, (CQItemType *)i2s0_loop_output_queue_buf);
    InitCQueue(&i2s1_loop_output_queue, I2S_LOOP_OUTPUT_QUEUE_BUFFER_SIZE, (CQItemType *)i2s1_loop_output_queue_buf);
    //i2s input , i2s output ,
    if(mode == 1)
    {
        //start_print_log = true;

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 /*| AUD_CHANNEL_MAP_CH2 | AUD_CHANNEL_MAP_CH3*/);
        //stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
        stream_cfg.sample_rate = I2S_LOOP_SAMPLE_RATE;
        stream_cfg.vol = 20;
        stream_cfg.device = out_dev;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = i2s0_output_cb;
        stream_cfg.data_ptr = i2s0_playback_buffer;
        stream_cfg.data_size = DAC_BUFFER_SIZE;
        ret = af_stream_open(I2S0_AUD_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
        TRACE(1, "%s: af_stream_open(playback) = %d", __func__, ret);
        ret = af_stream_start(I2S0_AUD_STREAM_ID, AUD_STREAM_PLAYBACK);
        TRACE(1, "%s: af_stream_start(playback) = %d", __func__, ret);


        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 /*| AUD_CHANNEL_MAP_CH2 | AUD_CHANNEL_MAP_CH3*/);
        //stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
        stream_cfg.sample_rate = I2S_LOOP_SAMPLE_RATE;
        stream_cfg.vol = 13;
        stream_cfg.device = in_dev;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = i2s0_input_cb;
        stream_cfg.data_ptr = i2s0_capture_buffer;
        stream_cfg.data_size = I2S_BUFFER_SIZE;
        ret = af_stream_open(I2S0_AUD_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        TRACE(1, "%s: af_stream_open(capture) = %d", __func__, ret);
        ret = af_stream_start(I2S0_AUD_STREAM_ID, AUD_STREAM_CAPTURE);
        TRACE(1, "%s: af_stream_start(capture) = %d", __func__, ret);
#if 1
        if(0)
        {
            // enable i2s1 output
            memset(&stream_cfg, 0, sizeof(stream_cfg));
            stream_cfg.bits = AUD_BITS_16;
            stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
            stream_cfg.sample_rate = I2S_LOOP_SAMPLE_RATE;
            stream_cfg.vol = 14;
            stream_cfg.device = out_dev;
            stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
            stream_cfg.handler = i2s1_output_cb;
            stream_cfg.data_ptr = i2s1_playback_buffer;
            stream_cfg.data_size = DAC_BUFFER_SIZE;
            af_stream_open(I2S1_AUD_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
            af_stream_start(I2S1_AUD_STREAM_ID, AUD_STREAM_PLAYBACK);

            // enable i2s1 input
            memset(&stream_cfg, 0, sizeof(stream_cfg));
            stream_cfg.bits = AUD_BITS_16;
            stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
            stream_cfg.sample_rate = I2S_LOOP_SAMPLE_RATE;
            stream_cfg.vol = 13;
            stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
            stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
            stream_cfg.handler = i2s1_input_cb;
            stream_cfg.data_ptr = i2s1_capture_buffer;
            stream_cfg.data_size = I2S_BUFFER_SIZE;
            af_stream_open(I2S1_AUD_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
            af_stream_start(I2S1_AUD_STREAM_ID, AUD_STREAM_CAPTURE);
        }
#endif
    }
}

void _i2s_loop_test(void)
{
    enum AUD_STREAM_USE_DEVICE_T in_dev, out_dev;

#if  defined(ADC_DAC_LOOP_TEST)
    in_dev = AUD_STREAM_USE_INT_CODEC;
    out_dev = AUD_STREAM_USE_INT_CODEC;
#elif defined(ADC_I2S_LOOP_MASTER_TEST)
    in_dev = AUD_STREAM_USE_INT_CODEC;
    out_dev = AUD_STREAM_USE_I2S0_MASTER;
#elif defined(I2S_DAC_LOOP_MASTER_TEST)
    in_dev = AUD_STREAM_USE_I2S0_MASTER;
    out_dev = AUD_STREAM_USE_INT_CODEC;
#elif defined(I2S_I2S_LOOP_MASTER_TEST)
    in_dev = AUD_STREAM_USE_I2S0_MASTER;
    out_dev = AUD_STREAM_USE_I2S0_MASTER;
#elif defined(ADC_I2S_LOOP_SLAVER_TEST)
    in_dev = AUD_STREAM_USE_INT_CODEC;
    out_dev = AUD_STREAM_USE_I2S0_SLAVE;
#elif defined(I2S_DAC_LOOP_SLAVER_TEST)
    in_dev = AUD_STREAM_USE_I2S0_SLAVE;
    out_dev = AUD_STREAM_USE_INT_CODEC;
#elif defined(I2S_I2S_LOOP_SLAVER_TEST)
    in_dev = AUD_STREAM_USE_I2S0_SLAVE;
    out_dev = AUD_STREAM_USE_I2S0_SLAVE;
#elif defined(TDM_DAC_LOOP_MASTER_TEST)
    in_dev = AUD_STREAM_USE_TDM0_MASTER;
    out_dev = AUD_STREAM_USE_INT_CODEC;
#elif defined(TDM_TDM_LOOP_MASTER_TEST)
    in_dev = AUD_STREAM_USE_TDM0_MASTER;
    out_dev = AUD_STREAM_USE_TDM0_MASTER;
#elif defined(ADC_TDM_LOOP_SLAVER_TEST)
    in_dev = AUD_STREAM_USE_INT_CODEC;
    out_dev = AUD_STREAM_USE_TDM0_SLAVE;
#elif defined(TDM_DAC_LOOP_SLAVER_TEST)
    in_dev = AUD_STREAM_USE_TDM0_SLAVE;
    out_dev = AUD_STREAM_USE_INT_CODEC;
#elif defined(TDM_I2S_LOOP_SLAVER_TEST)
    in_dev = AUD_STREAM_USE_TDM0_SLAVE;
    out_dev = AUD_STREAM_USE_TDM0_SLAVE;
#else // defualt I2S-I2S-MASTER
    in_dev = AUD_STREAM_USE_I2S0_MASTER;
    out_dev = AUD_STREAM_USE_I2S0_MASTER;
#endif
    _i2s_loopback_test(1, in_dev, out_dev);
    while(1){
        osDelay(10000);
        TRACE(1, "%s",__func__);
        };
}

#ifdef AD_I2S_LOOP_TEST
void _ad_i2s_loop_test(void)
{

    i2s_audio_on(true);
    while(1){
        i2s_audio_run();
    }
    while(1){
        //osDelay(1000);
        //TRACE(1, "%s",__func__);
        };
}
#endif

void function_test_i2s(void)
{
    af_open();
#ifdef AD_I2S_LOOP_TEST
    _ad_i2s_loop_test();
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
    _i2s_loop_test();
#endif

}
#endif

