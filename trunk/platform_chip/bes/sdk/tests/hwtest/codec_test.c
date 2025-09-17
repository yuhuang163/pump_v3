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
#include "hal_dma.h"
#include "hal_cmu.h"
#include "hal_codec.h"
#include "string.h"

// CODEC_SIMU
#define CODEC_IN_FRAME_SIZE (48 * 2 * 20)
#define CODEC_OUT_FRAME_SIZE (48 * 4 * 20)
static uint32_t codec_in_buf[2][CODEC_IN_FRAME_SIZE / 4];
static uint32_t codec_out_buf[2][CODEC_OUT_FRAME_SIZE / 4];
static struct HAL_DMA_DESC_T codec_in_dma_desc[2];
static struct HAL_DMA_DESC_T codec_out_dma_desc[2];

static void codec_in_full_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    int idx = (lli == &codec_in_dma_desc[0]) ? 0 : 1;
    uint16_t *src, *src_end;
    uint32_t *dst;

    TRACE(1,"codec_in_full_handler: in buffer %d finished", idx);

    src = (uint16_t *)&codec_in_buf[idx][0];
    src_end = (uint16_t *)&codec_in_buf[idx][CODEC_IN_FRAME_SIZE / 4];
    dst = (uint32_t *)&codec_out_buf[idx][0];

    while (src < src_end) {
        *dst++ = *src++ << 2;
    }

    TRACE(1,"codec_in_full_handler: out buffer %d ready", idx);
}

static void codec_out_full_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    int idx = (lli == &codec_out_dma_desc[0]) ? 0 : 1;

    TRACE(1,"codec_out_full_handler: out buffer %d finished", idx);
}

void codec_test(void)
{
    int i;
    struct HAL_DMA_CH_CFG_T codec_dma_cfg;
    struct HAL_CODEC_CONFIG_T codec_cfg;

    hal_codec_open(HAL_CODEC_ID_0);

    memset(&codec_cfg, 0, sizeof(codec_cfg));
    codec_cfg.bits = 16;
    codec_cfg.sample_rate = AUD_SAMPRATE_48000;
    codec_cfg.channel_num = 2;
    codec_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    codec_cfg.use_dma = true;
    codec_cfg.vol = 10;
    codec_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
    codec_cfg.set_flag = HAL_CODEC_CONFIG_ALL;
    hal_codec_setup_stream(HAL_CODEC_ID_0, AUD_STREAM_CAPTURE, &codec_cfg);

    memset(&codec_cfg, 0, sizeof(codec_cfg));
    codec_cfg.bits = 24;
    codec_cfg.sample_rate = AUD_SAMPRATE_48000;
    codec_cfg.channel_num = 2;
    codec_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    codec_cfg.use_dma = true;
    codec_cfg.vol = 10;
    codec_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
    codec_cfg.set_flag = HAL_CODEC_CONFIG_ALL;
    hal_codec_setup_stream(HAL_CODEC_ID_0, AUD_STREAM_PLAYBACK, &codec_cfg);

    // Setup codec in DMA
    memset(&codec_dma_cfg, 0, sizeof(codec_dma_cfg));
    codec_dma_cfg.dst_bsize = HAL_DMA_BSIZE_8;
    //codec_dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; //useless
    codec_dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    codec_dma_cfg.handler = codec_in_full_handler;
    codec_dma_cfg.src = 0; // useless
    codec_dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    codec_dma_cfg.src_periph = HAL_AUDMA_CODEC_RX;
    codec_dma_cfg.src_tsize = CODEC_IN_FRAME_SIZE / 2;
    codec_dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    codec_dma_cfg.try_burst = 1;
    codec_dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    codec_dma_cfg.ch = hal_audma_get_chan(codec_dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < 2; i++) {
        codec_dma_cfg.dst = (uint32_t)&codec_in_buf[i][0];
        hal_audma_init_desc(&codec_in_dma_desc[i], &codec_dma_cfg, &codec_in_dma_desc[(i + 1) % 2], 1);
    }
    hal_audma_sg_start(&codec_in_dma_desc[0], &codec_dma_cfg);

    // Setup codec out DMA
    memset(&codec_dma_cfg, 0, sizeof(codec_dma_cfg));
    codec_dma_cfg.dst = 0; // useless
    codec_dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    codec_dma_cfg.dst_periph = HAL_AUDMA_CODEC_TX;
    codec_dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    codec_dma_cfg.handler = codec_out_full_handler;
    codec_dma_cfg.src_bsize = HAL_DMA_BSIZE_16;
    //codec_dma_cfg.src_periph = HAL_AUDMA_PERIPH_QTY; // useless
    codec_dma_cfg.src_tsize = CODEC_OUT_FRAME_SIZE / 4;
    codec_dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    codec_dma_cfg.try_burst = 1;
    codec_dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    codec_dma_cfg.ch = hal_audma_get_chan(codec_dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < 2; i++) {
        codec_dma_cfg.src = (uint32_t)&codec_out_buf[i][0];
        hal_audma_init_desc(&codec_out_dma_desc[i], &codec_dma_cfg, &codec_out_dma_desc[(i + 1) % 2], 1);
    }
    hal_audma_sg_start(&codec_out_dma_desc[0], &codec_dma_cfg);

    hal_codec_start_interface(HAL_CODEC_ID_0, AUD_STREAM_CAPTURE, true);
    hal_codec_start_interface(HAL_CODEC_ID_0, AUD_STREAM_PLAYBACK, true);
    hal_codec_start_stream(HAL_CODEC_ID_0, AUD_STREAM_CAPTURE);
    hal_codec_start_stream(HAL_CODEC_ID_0, AUD_STREAM_PLAYBACK);
}


