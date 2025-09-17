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
#include "plat_addr_map.h"
#include "hal_dma.h"
#include "hal_aud.h"
#include "hal_spdif.h"
#include "hal_iomux.h"
#include "hal_cmu.h"
#include "string.h"

#ifdef SPDIF0_BASE

#define SPDIF_FRAME_SIZE 64
#define SPDIF_FRAME_NUM 4
#define SPDIF_BUF_ALIGN __attribute__((aligned(0x100)))
static uint32_t SPDIF_BUF_ALIGN spdif_buf[SPDIF_FRAME_NUM][SPDIF_FRAME_SIZE / 4];
static struct HAL_DMA_DESC_T spdif_rx_dma_desc[SPDIF_FRAME_NUM];
static struct HAL_DMA_DESC_T spdif_tx_dma_desc[SPDIF_FRAME_NUM];

static void spdif_tx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % 60 == 0) {
        TRACE(3,"SPDIF-TX: remains=%d, error=%d, cnt=%d", remains, error, cnt);
    }
}

static void spdif_rx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % 60 == 0) {
        TRACE(3,"SPDIF-RX: remains=%d, error=%d, cnt=%d", remains, error, cnt);
    }
}

void spdif_test(void)
{
    int i;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    struct HAL_SPDIF_CONFIG_T spdif_cfg;
    int ret;

    TRACE(0,"SPDIF: Start configuration");

    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);

    ret = hal_spdif_open(HAL_SPDIF_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "Failed to open spdif playback");
    ret = hal_spdif_open(HAL_SPDIF_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "Failed to open spdif capture");

    spdif_cfg.use_dma = 1;
    spdif_cfg.bits = 16;
    spdif_cfg.channel_num = 2;
    spdif_cfg.sample_rate = AUD_SAMPRATE_48000;
    ret = hal_spdif_setup_stream(HAL_SPDIF_ID_0, AUD_STREAM_PLAYBACK, &spdif_cfg);
    ASSERT(ret == 0, "Failed to setup spdif tx stream");
    ret = hal_spdif_setup_stream(HAL_SPDIF_ID_0, AUD_STREAM_CAPTURE, &spdif_cfg);
    ASSERT(ret == 0, "Failed to setup spdif rx stream");

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; //useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = HAL_AUDMA_SPDIF0_TX;
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = spdif_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.src_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.src_tsize = sizeof(spdif_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < SPDIF_FRAME_NUM; i++) {
        dma_cfg.src = (uint32_t)&spdif_buf[i][0];
        ret = hal_audma_init_desc(&spdif_tx_dma_desc[i], &dma_cfg, &spdif_tx_dma_desc[(i + 1) % SPDIF_FRAME_NUM], (i + 1) % (SPDIF_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for tx");
    }

    ret = hal_audma_sg_start(&spdif_tx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for tx");

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = spdif_rx_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_SPDIF0_RX;
    dma_cfg.src_tsize = sizeof(spdif_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < SPDIF_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&spdif_buf[(i + 1) % SPDIF_FRAME_NUM][0];
        ret = hal_audma_init_desc(&spdif_rx_dma_desc[i], &dma_cfg, &spdif_rx_dma_desc[(i + 1) % SPDIF_FRAME_NUM], (i + 1) % (SPDIF_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for rx");
    }

    ret = hal_audma_sg_start(&spdif_rx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for rx");

    ret = hal_spdif_start_stream(HAL_SPDIF_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "Failed to start spdif tx stream");

    ret = hal_spdif_start_stream(HAL_SPDIF_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "Failed to start spdif rx stream");

    TRACE(0,"SPDIF: Configuration done");
}

#endif
