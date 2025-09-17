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
#include "hal_aud.h"
#include "hal_i2s.h"
#include "hal_iomux.h"
#include "hal_cmu.h"
#include "string.h"

/* tanwenchen add,2020-11-18 */
//#define I2S_DATA_OUTPUT

//mcu i2s id and number config
#define MCU_I2S_ID          HAL_I2S_ID_0
#define MCU_I2S_TX_NO       HAL_AUDMA_I2S0_TX
#define MCU_I2S_RX_NO       HAL_AUDMA_I2S0_RX
//sens i2s id and number config
#define SENS_I2S_ID         HAL_I2S_ID_0
#define SENS_I2S_TX_NO      HAL_AUDMA_I2S0_TX
#define SENS_I2S_RX_NO      HAL_AUDMA_I2S0_RX


#define I2S_FRAME_SIZE 64
#define I2S_FRAME_NUM 2
#define I2S_BUF_ALIGN __attribute__((aligned(0x100)))
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

void i2s_test(void)
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

    //ret = hal_iomux_init(pinmux_i2s, sizeof(pinmux_i2s)/sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
    //ASSERT(ret == 0, "Failed to init iomux");

#ifdef CHIP_SUBSYS_SENS
    ret = hal_i2s_open(SENS_I2S_ID, AUD_STREAM_PLAYBACK, HAL_I2S_MODE_SLAVE);
    ASSERT(ret == 0, "Sens Failed to open i2s tx master");

    ret = hal_i2s_open(SENS_I2S_ID, AUD_STREAM_CAPTURE, HAL_I2S_MODE_SLAVE);
    ASSERT(ret == 0, "Sens Failed to open i2s rx master");
#else
    ret = hal_i2s_open(MCU_I2S_ID, AUD_STREAM_PLAYBACK, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open i2s tx master");

    ret = hal_i2s_open(MCU_I2S_ID, AUD_STREAM_CAPTURE, HAL_I2S_MODE_MASTER);
    ASSERT(ret == 0, "Failed to open i2s rx master");
#endif

    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.use_dma = true;
    i2s_cfg.sync_start = true;
    i2s_cfg.chan_sep_buf = false;
    i2s_cfg.bits = 16;
    i2s_cfg.channel_num = 2;
    i2s_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    i2s_cfg.sample_rate = AUD_SAMPRATE_192000;
#ifdef CHIP_SUBSYS_SENS
    ret = hal_i2s_setup_stream(SENS_I2S_ID, AUD_STREAM_PLAYBACK, &i2s_cfg);
#else
    ret = hal_i2s_setup_stream(MCU_I2S_ID, AUD_STREAM_PLAYBACK, &i2s_cfg);
#endif
    ASSERT(ret == 0, "Failed to setup i2s tx stream");
    i2s_cfg.sync_start = false;
#ifdef CHIP_SUBSYS_SENS
    ret = hal_i2s_setup_stream(SENS_I2S_ID, AUD_STREAM_CAPTURE, &i2s_cfg);
#else
    ret = hal_i2s_setup_stream(MCU_I2S_ID, AUD_STREAM_CAPTURE, &i2s_cfg);
#endif
    ASSERT(ret == 0, "Failed to setup i2s rx stream");

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; //useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;

#ifdef CHIP_SUBSYS_SENS
    dma_cfg.dst_periph = SENS_I2S_TX_NO;
#else
    dma_cfg.dst_periph = MCU_I2S_TX_NO;
#endif  //CHIP_SUBSYS_SENS
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = i2s_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    //dma_cfg.src_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.src_tsize = sizeof(i2s_tx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
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
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = i2s_rx_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
#ifdef CHIP_SUBSYS_SENS
    dma_cfg.src_periph = SENS_I2S_RX_NO;
#else
    dma_cfg.src_periph = MCU_I2S_RX_NO;
#endif  //CHIP_SUBSYS_SENS
    dma_cfg.src_tsize = sizeof(i2s_rx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
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

#ifdef CHIP_SUBSYS_SENS
    ret = hal_i2s_start_stream(SENS_I2S_ID, AUD_STREAM_PLAYBACK);
#else
    ret = hal_i2s_start_stream(MCU_I2S_ID, AUD_STREAM_PLAYBACK);
#endif
    ASSERT(ret == 0, "Failed to start i2s tx stream");

#ifdef CHIP_SUBSYS_SENS
    ret = hal_i2s_start_stream(SENS_I2S_ID, AUD_STREAM_CAPTURE);
#else
    ret = hal_i2s_start_stream(MCU_I2S_ID, AUD_STREAM_CAPTURE);
#endif
    ASSERT(ret == 0, "Failed to start i2s rx stream");

    TRACE(0,"I2S: Configuration done");
}

