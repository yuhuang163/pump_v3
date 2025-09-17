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
#ifdef TEST_I2S
#include "hal_dma.h"
#include "hal_i2s.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "string.h"


#define I2S_DATA_OUTPUT
#define I2S_BITS            16
#define I2S_AUD_SAMPRATE    AUD_SAMPRATE_16000
#define HAL_DMA_SIZE        ((32 == I2S_BITS) ? HAL_DMA_WIDTH_WORD : HAL_DMA_WIDTH_HALFWORD)
#define I2S_MODE            HAL_I2S_MODE_MASTER

// i2s id and number config
#if 1
#define I2S_ID              HAL_I2S_ID_0
#define I2S_TX_NO           HAL_AUDMA_I2S0_TX
#define I2S_RX_NO           HAL_AUDMA_I2S0_RX
#else
#define I2S_ID              HAL_I2S_ID_1
#define I2S_TX_NO           HAL_AUDMA_I2S1_TX
#define I2S_RX_NO           HAL_AUDMA_I2S1_RX
#endif

#define I2S_FRAME_SIZE      64
#define I2S_FRAME_NUM       2
#define I2S_BUF_ALIGN       __attribute__((aligned(0x100)))
static uint32_t I2S_BUF_ALIGN i2s_rx_buf[I2S_FRAME_NUM][I2S_FRAME_SIZE / 4];
static uint32_t I2S_BUF_ALIGN i2s_tx_buf[I2S_FRAME_NUM][I2S_FRAME_SIZE / 4];
static struct HAL_DMA_DESC_T i2s_rx_dma_desc[I2S_FRAME_NUM];
static struct HAL_DMA_DESC_T i2s_tx_dma_desc[I2S_FRAME_NUM];

//static struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_i2s[] = {
//};

static void i2s_tx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % 60 == 0) {
        TRACE(3,"I2S-TX: remains=%d, error=%d, cnt=%d", remains, error, cnt);

#ifdef  I2S_DATA_OUTPUT
        TRACE(TR_ATTR_NO_LF,"I2S-TX data:");
        for(uint8_t i=0;i<I2S_FRAME_SIZE / 4;i++)
        {
            TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"0x%08X ", i2s_tx_buf[0][i]);
        }
        TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"\r\n");
        TRACE(TR_ATTR_NO_LF,"I2S-TX data:");
        for(uint8_t i=0;i<I2S_FRAME_SIZE / 4;i++)
        {
            TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"0x%08X ", i2s_tx_buf[1][i]);
        }
        TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"\r\n");
#endif
    }
}

static void i2s_rx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % 60 == 0) {
        TRACE(3,"I2S-RX: remains=%d, error=%d, cnt=%d", remains, error, cnt);

#ifdef  I2S_DATA_OUTPUT
        TRACE(TR_ATTR_NO_LF,"I2S-RX data:");
        for(uint8_t i=0;i<I2S_FRAME_SIZE / 4;i++)
        {
            TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"0x%08X ", i2s_rx_buf[0][i]);
        }
        TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"\r\n");
        TRACE(TR_ATTR_NO_LF,"I2S-RX data:");
        for(uint8_t i=0;i<I2S_FRAME_SIZE / 4;i++)
        {
            TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"0x%08X ", i2s_rx_buf[1][i]);
        }
        TRACE(TR_ATTR_NO_LF|TR_ATTR_NO_TS|TR_ATTR_NO_ID,"\r\n");
#endif
    }
}

void test_i2s(void)
{
    int i;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    struct HAL_I2S_CONFIG_T i2s_cfg;
    int ret;
    uint8_t *buf;

    buf = (uint8_t *)&i2s_tx_buf[0];
    for (i = 0; i < I2S_FRAME_NUM * I2S_FRAME_SIZE; i++) {
        buf[i] = (uint8_t)i;
    }

    TRACE(0,"I2S: Start configuration");

    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);

//     if(HAL_I2S_MODE_MASTER == I2S_MODE) {
// #ifdef I2S_MCLK_IN
//         hal_iomux_set_i2s0_mclk_in();
// #else
//         hal_iomux_set_i2s0_mclk();
// #endif
//     }

    ret = hal_i2s_open(I2S_ID, AUD_STREAM_PLAYBACK, I2S_MODE);
    ASSERT(ret == 0, "Sens Failed to open i2s tx master");

    ret = hal_i2s_open(I2S_ID, AUD_STREAM_CAPTURE, I2S_MODE);
    ASSERT(ret == 0, "Sens Failed to open i2s rx master");

    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.use_dma = true;
    i2s_cfg.sync_start = true;
    i2s_cfg.chan_sep_buf = false;
    i2s_cfg.bits = I2S_BITS;
    i2s_cfg.channel_num = 2;
    i2s_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    i2s_cfg.sample_rate = I2S_AUD_SAMPRATE;
    i2s_cfg.ext_mclk_freq = 24576000;

    ret = hal_i2s_setup_stream(I2S_ID, AUD_STREAM_PLAYBACK, &i2s_cfg);
    ASSERT(ret == 0, "Failed to setup i2s tx stream");

    i2s_cfg.sync_start = false;
    ret = hal_i2s_setup_stream(I2S_ID, AUD_STREAM_CAPTURE, &i2s_cfg);
    ASSERT(ret == 0, "Failed to setup i2s rx stream");

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; //useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = I2S_TX_NO;
    dma_cfg.dst_width = HAL_DMA_SIZE;
    dma_cfg.handler = i2s_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_tsize = sizeof(i2s_tx_buf[0]) / (I2S_BITS / 8);
    dma_cfg.src_width = HAL_DMA_SIZE;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < I2S_FRAME_NUM; i++) {
        dma_cfg.src = (uint32_t)&i2s_tx_buf[i][0];
        ret = hal_audma_init_desc(&i2s_tx_dma_desc[i], &dma_cfg, &i2s_tx_dma_desc[(i + 1) % I2S_FRAME_NUM], (i + 1) % (I2S_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for tx");
    }

    ret = hal_audma_sg_start(&i2s_tx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for tx");

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_width = HAL_DMA_SIZE;
    dma_cfg.handler = i2s_rx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = I2S_RX_NO;
    dma_cfg.src_tsize = sizeof(i2s_rx_buf[0]) / (I2S_BITS / 8);
    dma_cfg.src_width = HAL_DMA_SIZE;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < I2S_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&i2s_rx_buf[i][0];
        ret = hal_audma_init_desc(&i2s_rx_dma_desc[i], &dma_cfg, &i2s_rx_dma_desc[(i + 1) % I2S_FRAME_NUM], (i + 1) % (I2S_FRAME_NUM / 2) == 0);
        ASSERT(ret == 0, "Failed to init dma desc for rx");
    }

    ret = hal_audma_sg_start(&i2s_rx_dma_desc[0], &dma_cfg);
    ASSERT(ret == 0, "Failed to start dma sg for rx");

    ret = hal_i2s_start_stream(I2S_ID, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "Failed to start i2s tx stream");

    ret = hal_i2s_start_stream(I2S_ID, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "Failed to start i2s rx stream");

    TRACE(0,"I2S: Configuration done");
}


#endif


