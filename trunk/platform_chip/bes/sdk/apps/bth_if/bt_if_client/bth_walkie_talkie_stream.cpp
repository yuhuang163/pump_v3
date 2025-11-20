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
#ifdef BLE_WALKIE_TALKIE
#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "audioflinger.h"
#include "app_utils.h"
#include "app_overlay.h"
#include "app_audio.h"
#include "kfifo.h"
#include "smf_codec_opus.h"
#include "smf_opus_defines.h"
#include "SmfFCC.h"
#include "smf_debug.h"
#include "bth_walkie_talkie_stream.h"
#include "bth_bt_if_client.h"
#include "bth_if_evt_def.h"
#include "bth_if_cmd_def.h"
#include "hal_gpio.h"
#include "bt_sco_chain.h"

#include "audio_dump.h"
#ifdef UTILS_ESHELL_EN
#include "eshell.h"
#endif

//#define WT_AF_STREAM_LOOP_TEST
//#define BTH_WT_AUDIO_DUMP 1
#define OUTPUT_MAX     (960)
//#define CAPTURE_USE_DIG_MIC

#define BT_AUDIO_CACHE_2_UNCACHE(addr) \
    ((unsigned char *)((unsigned int)addr & ~(0x04000000)))

void* encoder_frame = NULL;
void* encoder_handle = NULL;
void* smf_decoder = NULL;
void* smf_decoder_param = NULL;
uint8_t *p_capture_dma_buf_ptr = NULL;

#define WT_CAPTURE_AUDIO_KFIFO_SIZE  1024 * 4
static SRAM_BSS_LOC struct kfifo wt_capture_kfifo;
static SRAM_BSS_LOC uint8_t capture_kfifo_buffer[WT_CAPTURE_AUDIO_KFIFO_SIZE];
static uint8_t capture_frame_outbuff[83] = {0};
#ifdef WT_AF_STREAM_LOOP_TEST
static SRAM_BSS_LOC uint8_t play[4096]; __attribute__((aligned(4)))
#endif
static SRAM_BSS_LOC uint8_t capture[2048];__attribute__((aligned(4)))

#ifdef CAPTURE_USE_DIG_MIC
static void bt_sco_digital_mic_volt_enable(bool enable)
{
    uint32_t gpiopin;
    struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_gpio;

    gpiopin = HAL_GPIO_PIN_PMU_P0_0 + 15 / 10 * 8 + 15 % 10;  //P15

    cfg_hw_gpio.pin = (enum HAL_IOMUX_PIN_T) gpiopin;
    cfg_hw_gpio.function = HAL_IOMUX_FUNC_AS_GPIO;
    cfg_hw_gpio.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
    cfg_hw_gpio.pull_sel = HAL_IOMUX_PIN_PULLUP_ENABLE;
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_hw_gpio, 1);

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_hw_gpio.pin, HAL_GPIO_DIR_OUT, enable);
}
#endif

static void wt_audio_opus_encoder_init(void)
{
    TRACE(0, "%s", __func__);
    smf_init();

    smf_opus_encoder_register();
#ifdef WT_AF_STREAM_LOOP_TEST
    smf_opus_decoder_register();
#endif
    encoder_handle = smf_create_encoder("opus");
    returnIfErrC0(!encoder_handle);
    static smf_opus_enc_open_param_t param;
    param.media.codec = smf::fcc64("opus");
    param.application = OPUS_APPLICATION_VOIP;
    param.complexity = 0;
    param.lsb_depth = 16;
    param.media.bitrate = 11000;
    param.media.audio.channels = 1;
    param.media.audio.sample_rate = 8000;
    param.media.frame_dms = 600 ;    //600 means 60ms
    param.media.audio.sample_bits = 16;
    param.media.audio.sample_width = 2;
    if (!smf_open(encoder_handle, &param))
    {
        smf_print_error(encoder_handle);
        smf_free(encoder_handle, smf_decoder_param);
        smf_destroy(encoder_handle);
        return;
    }

#ifdef WT_AF_STREAM_LOOP_TEST
    smf_decoder = smf_create_decoder("opus");
    if (!smf_decoder)
    {
        smf_print_error(smf_decoder);
        return;
    }

    smf_decoder_param = smf_alloc_open_param(smf_decoder);
    if (!smf_decoder_param)
    {
        smf_print_error(smf_decoder);
        smf_destroy(smf_decoder);
        return ;
    }

    smf_media_info_t* decparam = (smf_media_info_t*)smf_decoder_param;
    decparam->audio.channels = 1;
    decparam->audio.sample_rate = 8000;
    decparam->audio.sample_bits = 16;
    decparam->audio.frame_samples = 83;
    decparam->bitrate = 11000;
    decparam->frame_size = 83;
    if (false == smf_open(smf_decoder, smf_decoder_param))
    {
        smf_print_error(smf_decoder);
        smf_free(smf_decoder, smf_decoder_param);
        smf_destroy(smf_decoder);
    }
#endif

    TRACE(1, "%s end", __func__);
}

static void wt_audio_opus_encoder_deinit(void)
{
    smf_close(encoder_handle);
    smf_destroy(encoder_handle);
    encoder_handle = NULL;

    TRACE(0, "opus encode close.");
}

static bool wt_audio_opus_encode(uint8_t* in_buf, uint8_t* out_buf, uint32_t length)
{
    bool ret;

    smf_frame_t out_frm;
    smf_frame_t i_frm;
    memset(&i_frm, 0, sizeof(smf_frame_t));
    memset(&out_frm, 0, sizeof(smf_frame_t));
    i_frm.buff = in_buf;
    i_frm.max = length;
    i_frm.size = length;
    i_frm.offset = 0;
    i_frm.flags = 0;
    out_frm.size = 0;
    out_frm.offset = 0;
    out_frm.max = OUTPUT_MAX;
    out_frm.buff = out_buf;
    ret = smf_encode(encoder_handle, &i_frm, &out_frm);

    //LOG_I("%s end,ret = %d", __func__,ret);

    return ret;
}

POSSIBLY_UNUSED static bool wt_audio_opus_decode(uint8_t* inbuf,uint16_t length,void* output)
{
    bool ret;
    smf_frame_t out_frm;
    smf_frame_t i_frm;
    memset(&i_frm, 0, sizeof(smf_frame_t));
    i_frm.buff = inbuf;
    i_frm.max = length;
    i_frm.size = length;
    i_frm.offset = 0;
    i_frm.flags = 0;
    out_frm.offset = 0;
    out_frm.size = 0;
    out_frm.buff = output;
    out_frm.max = 960;

    ret = smf_decode(smf_decoder, &i_frm, &out_frm);
    TRACE(2, "%s out size %d ret %d", __func__, out_frm.size, ret);
    return ret;
}


POSSIBLY_UNUSED static void wt_audio_opus_encoder_buf_deinit(void* p_stream_env)
{
    TRACE(0, "%s",__func__);
}

int wt_stream_kfifo_put_data(uint8_t *in_buf, uint32_t len)
{
    unsigned int freeSpace = 0;

    freeSpace = kfifo_get_free_space(&wt_capture_kfifo);
    if (freeSpace < len)
    {
        //TRACE(1,"kfifo overflow!!! free space = %d", freeSpace);
        return false;
    }
    else
    {
        kfifo_put(&wt_capture_kfifo, in_buf, len);
    }

    return true;
}

#ifdef WT_AF_STREAM_LOOP_TEST
#include "cqueue.h"
CQueue capture_queue;
static uint8_t capture_queue_buffer[WT_CAPTURE_AUDIO_KFIFO_SIZE];
#endif

static uint32_t wt_capture_data_dma_irq_handler(uint8_t* ptr_buf, uint32_t length)
{
    //wt_audio_opus_encode
    if (true == wt_audio_opus_encode(ptr_buf, capture_frame_outbuff, length))
    {
        TRACE(2, "%s capture data len %d", __func__, length);
        wt_stream_kfifo_put_data(capture_frame_outbuff, 83);
#ifdef WT_AF_STREAM_LOOP_TEST
        // EnCQueue(&capture_queue, capture_frame_outbuff, 83);
        // DUMP8("%02x", capture_frame_outbuff, 83);
        EnCQueue(&capture_queue, ptr_buf, length);
#endif

#ifdef  BTH_WT_AUDIO_DUMP
        audio_dump_clear_up();
        audio_dump_add_channel_data(0, capture_frame_outbuff, 83);
        audio_dump_run();
#endif
    }
    else
    {
        TRACE(0, "opus encode error!!!");
    }

    return length;
}

#ifdef WT_AF_STREAM_LOOP_TEST

static void wt_stream_copy_track_one_to_two_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t src_len)
{
    // Copy from tail so that it works even if dst_buf == src_buf
    for (int i = (int)(src_len - 1); i >= 0; i--)
    {
        dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i];
    }
}

uint8_t playback_buffer[2048];
uint8_t *p_playback_dma_buf_ptr = NULL;
static uint32_t wt_playback_data_dma_irq_handler(uint8_t* ptr_buf, uint32_t length)
{
    //pcm loop
    uint32_t len = length / 2;
    uint16_t * p_pcm, *p_capture;
    uint32_t available_len =  LengthOfCQueue(&capture_queue);
    if(available_len >= len)
    {
        TRACE(2, "%s length %d", __func__, length);

        DeCQueue(&capture_queue, playback_buffer, len);
        p_pcm = (uint16_t *)ptr_buf;
        p_capture = (uint16_t *)playback_buffer;
        for(uint32_t i = 0; i < (len/2); i++)
        {
            p_pcm[i] = p_capture[i];
        }
        //one channel ---> two channel
        wt_stream_copy_track_one_to_two_16bits((int16_t *)p_pcm, (int16_t *)p_pcm, length / 2 / sizeof(int16_t));
    }

    // uint32_t available_len =  LengthOfCQueue(&capture_queue);
    // if(available_len >= 83)
    // {
    //     TRACE(2, "%s length %d", __func__, length);
    //     DeCQueue(&capture_queue, playback_buffer, 83);
    //     wt_audio_opus_decode(playback_buffer, 83, ptr_buf);
    //     wt_stream_copy_track_one_to_two_16bits((int16_t *)ptr_buf, (int16_t *)ptr_buf, length / 2 / sizeof(int16_t));
    // }
     return length;
}
#endif

int wt_audio_capture_stream_start(void)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    uint32_t dma_chunk_size = 0;
    app_ble_walkie_talkie_start_capture_cmd_t capture_kfifo;

    app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_208M);

    wt_audio_opus_encoder_init();
    kfifo_init(&wt_capture_kfifo, capture_kfifo_buffer, WT_CAPTURE_AUDIO_KFIFO_SIZE);
    capture_kfifo.kfifo =(void *)&wt_capture_kfifo;
    app_bth_ble_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_START_CAPTURE,(uint8_t *)&capture_kfifo,
                sizeof(app_ble_walkie_talkie_start_capture_cmd_t));

#ifdef CAPTURE_USE_DIG_MIC
    bt_sco_digital_mic_volt_enable(true);
#endif
    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.sample_rate = AUD_SAMPRATE_8000;
#ifdef CAPTURE_USE_DIG_MIC
    stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)AUD_CHANNEL_MAP_DIGMIC_CH0;
#else
    stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)AUD_CHANNEL_MAP_CH0;
#endif
    stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
    stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
    stream_cfg.vol = TGT_ADC_VOL_LEVEL_MAX;
    dma_chunk_size = (uint32_t)(stream_cfg.sample_rate * (stream_cfg.bits / 8) * 60 * \
                            stream_cfg.channel_num / (1000));
    stream_cfg.data_ptr = capture;
    stream_cfg.data_size = (uint32_t)(2 * dma_chunk_size);
    stream_cfg.handler = wt_capture_data_dma_irq_handler;
    af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

#ifdef WT_AF_STREAM_LOOP_TEST
    //stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)AUD_CHANNEL_MAP_CH0;
    stream_cfg.device = AUD_STREAM_USE_I2S1_MASTER;
    stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
    stream_cfg.vol = TGT_VOLUME_LEVEL_MAX;
    stream_cfg.data_ptr = play;
    stream_cfg.data_size = (uint32_t)(2 * 2 * dma_chunk_size);
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.handler = wt_playback_data_dma_irq_handler;
    af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
#endif

    af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
#ifdef WT_AF_STREAM_LOOP_TEST
    af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    InitCQueue(&capture_queue, WT_CAPTURE_AUDIO_KFIFO_SIZE, (unsigned char *)capture_queue_buffer);
#endif
#ifdef BTH_WT_AUDIO_DUMP
    audio_dump_init(83, 2, 1);
#endif

    return 0;
}


int wt_audio_capture_stream_stop(void)
{
#ifdef CAPTURE_USE_DIG_MIC
    bt_sco_digital_mic_volt_enable(false);
#endif
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

    wt_audio_opus_encoder_deinit();

    app_bth_ble_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_STOP_CAPTURE,NULL, 0);

    return 0;
}

#ifdef UTILS_ESHELL_EN
static void eshell_wt_stream_capture_start(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_wt_stream_capture_start... %d", argc);
    if (argc < 1) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    wt_audio_capture_stream_start();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_wt_start_capture",
                    "wt stream capture start",eshell_wt_stream_capture_start);

static void eshell_wt_stream_capture_stop(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_wt_stream_capture_stop... %d", argc);
    if (argc < 1) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    wt_audio_capture_stream_stop();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_wt_stop_capture",
                    "wt stream capture stop",eshell_wt_stream_capture_stop);

static void eshell_wt_stream_open_scan(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_wt_stream_open_scan... %d", argc);
    if (argc < 1) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    app_bth_ble_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_OPEN_SCAN, NULL, 0);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_wt_open_scan",
                    "wt stream open scan",eshell_wt_stream_open_scan);

#endif

#endif
