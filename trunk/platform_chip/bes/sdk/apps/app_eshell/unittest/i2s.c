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
#ifdef UTILS_ESHELL_EN
#if defined(__SYS_AS_MAIN__) || defined(CHIP_SUBSYS_SENS)

#include "stdlib.h"
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "eshell.h"
#include "hal_dma.h"
#include "hal_i2s.h"
#include "hal_timer.h"

#define I2S_FRAME_SIZE 64
#define I2S_FRAME_NUM 2
#define I2S_BUF_ALIGN __attribute__((aligned(0x100)))
static uint32_t I2S_BUF_ALIGN i2s_tx_buf[I2S_FRAME_NUM][I2S_FRAME_SIZE / 4];
static struct HAL_DMA_DESC_T i2s_tx_dma_desc[I2S_FRAME_NUM];
static uint32_t I2S_BUF_ALIGN i2s_rx_buf[I2S_FRAME_NUM][I2S_FRAME_SIZE / 4];
static struct HAL_DMA_DESC_T i2s_rx_dma_desc[I2S_FRAME_NUM];


static void i2s_tx_handler(uint8_t chan, uint32_t remains, uint32_t error,
                           struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;
    cnt++;
    if (cnt % 60 == 0) {
        eshell_putstring("I2S-TX: chan=%d, remains=%d, error=%d, cnt=%d\r\n", chan, remains, error,
                         cnt);

        eshell_putstring("I2S-TX-0 data:\r\n");
        for (uint8_t i = 0; i < I2S_FRAME_SIZE / 4; i++) {
            eshell_putstring("0x%08X ", i2s_tx_buf[0][i]);
        }

        eshell_putstring("\r\nI2S-TX-1 data:\r\n");
        for (uint8_t i = 0; i < I2S_FRAME_SIZE / 4; i++) {
            eshell_putstring("0x%08X ", i2s_tx_buf[1][i]);
        }
        eshell_putstring("\r\n");
    }
    //eshell_putstring("tx cnt = %d\r\n", cnt);
}

static void i2s_rx_handler(uint8_t chan, uint32_t remains, uint32_t error,
                           struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    cnt++;
    if (cnt % 60 == 0) {
        eshell_putstring("I2S-RX: chan=%d remains=%d, error=%d, cnt=%d\r\n", chan, remains, error, cnt);
        eshell_putstring("I2S-RX-0 data:\r\n");
        for (uint8_t i = 0; i < I2S_FRAME_SIZE / 4; i++) {
            eshell_putstring("0x%08X ", i2s_rx_buf[0][i]);
            if(i2s_rx_buf[0][i] != i2s_tx_buf[0][i]){
                eshell_putstring("i2s rx recv data error\r\n");
            }
        }
        eshell_putstring("\r\nI2S-RX-1 data:\r\n");
        for (uint8_t i = 0; i < I2S_FRAME_SIZE / 4; i++) {
            eshell_putstring("0x%08X ", i2s_rx_buf[1][i]);
            if(i2s_rx_buf[1][i] != i2s_tx_buf[1][i]){
                eshell_putstring("i2s rx recv data error\r\n");
            }
        }
        eshell_putstring("\r\n");
    }
    //eshell_putstring("rx cnt = %d\r\n", cnt);
}

static void i2s_start_stream(enum HAL_I2S_ID_T i2s_id, enum AUD_STREAM_T AUD_STREAM)
{
    if (AUD_STREAM_PLAYBACK == AUD_STREAM) {
        hal_i2s_start_stream(i2s_id, AUD_STREAM_PLAYBACK);
    } else if (AUD_STREAM_CAPTURE == AUD_STREAM) {
        hal_i2s_start_stream(i2s_id, AUD_STREAM_CAPTURE);
    }
}

void i2s_loop_test(enum HAL_I2S_ID_T i2s_id, uint32_t sample_rate, uint8_t i2s_bits)
{
    hal_i2s_open(i2s_id, AUD_STREAM_CAPTURE, HAL_I2S_MODE_MASTER);
    struct HAL_I2S_CONFIG_T i2s_cfg;
    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.use_dma = true;
    i2s_cfg.sync_start = false;
    i2s_cfg.chan_sep_buf = false;
    i2s_cfg.bits = i2s_bits;
    i2s_cfg.channel_num = 2;
    i2s_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    i2s_cfg.sample_rate = sample_rate;
    // i2s_cfg.ext_mclk_freq = 24576000;
    hal_i2s_setup_stream(i2s_id, AUD_STREAM_CAPTURE, &i2s_cfg);
    //dma config
    struct HAL_DMA_CH_CFG_T dma_cfg;
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = i2s_rx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_I2S0_RX;
    dma_cfg.src_tsize = sizeof(i2s_tx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);
    for (int i = 0; i < I2S_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&i2s_rx_buf[i][0];
        hal_audma_init_desc(&i2s_rx_dma_desc[i], &dma_cfg,
                            &i2s_rx_dma_desc[(i + 1) % I2S_FRAME_NUM],
                            (i + 1) % (I2S_FRAME_NUM / 2) == 0);
    }

    hal_audma_sg_start(&i2s_rx_dma_desc[0], &dma_cfg);
    eshell_putstring("I2S: Capture Configuration done\r\n");

    uint8_t *buf;
    buf = (uint8_t *)&i2s_tx_buf[0];
    for (int i = 0; i < I2S_FRAME_NUM * I2S_FRAME_SIZE; i++) {
        buf[i] = (uint8_t)i;
    }

    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);
    hal_i2s_open(i2s_id, AUD_STREAM_PLAYBACK, HAL_I2S_MODE_MASTER);

    i2s_cfg.sync_start = true;
    hal_i2s_setup_stream(i2s_id, AUD_STREAM_PLAYBACK, &i2s_cfg);

    hal_dma_open();

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; // useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = HAL_AUDMA_I2S0_TX;
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.handler = i2s_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_tsize = sizeof(i2s_tx_buf[0]) / 2;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (int i = 0; i < I2S_FRAME_NUM; i++) {
        dma_cfg.src = (uint32_t)&i2s_tx_buf[i][0];
        hal_audma_init_desc(&i2s_tx_dma_desc[i], &dma_cfg,
                            &i2s_tx_dma_desc[(i + 1) % I2S_FRAME_NUM],
                            (i + 1) % (I2S_FRAME_NUM / 2) == 0);
    }

    hal_audma_sg_start(&i2s_tx_dma_desc[0], &dma_cfg);
    eshell_putstring("I2S: Playback Configuration done\r\n");
    hal_i2s_start_stream(i2s_id, AUD_STREAM_PLAYBACK);
    hal_i2s_start_stream(i2s_id, AUD_STREAM_CAPTURE);
}

static void test_i2s_playback(enum HAL_I2S_ID_T i2s_id, enum AUD_BITS_T i2s_bit,
                              uint32_t sample_rate, enum HAL_DMA_WIDTH_T dma_width,
                              uint16_t dma_tsize)
{
    uint8_t *buf;

    buf = (uint8_t *)&i2s_tx_buf[0];
    for (int i = 0; i < I2S_FRAME_NUM * I2S_FRAME_SIZE; i++) {
        buf[i] = (uint8_t)i;
    }

    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);
    hal_i2s_open(i2s_id, AUD_STREAM_PLAYBACK, HAL_I2S_MODE_MASTER);

    struct HAL_I2S_CONFIG_T i2s_cfg;
    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.use_dma = true;
    i2s_cfg.sync_start = false;
    i2s_cfg.chan_sep_buf = false;
    i2s_cfg.bits = i2s_bit;
    i2s_cfg.channel_num = 2;
    i2s_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    i2s_cfg.sample_rate = sample_rate;
    i2s_cfg.ext_mclk_freq = 24576000;

    hal_i2s_setup_stream(i2s_id, AUD_STREAM_PLAYBACK, &i2s_cfg);

    hal_dma_open();

    struct HAL_DMA_CH_CFG_T dma_cfg;
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst = 0; // useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = HAL_AUDMA_I2S0_TX;
    dma_cfg.dst_width = dma_width;
    dma_cfg.handler = i2s_tx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_tsize = dma_tsize;
    dma_cfg.src_width = dma_width;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    for (int i = 0; i < I2S_FRAME_NUM; i++) {
        dma_cfg.src = (uint32_t)&i2s_tx_buf[i][0];
        hal_audma_init_desc(&i2s_tx_dma_desc[i], &dma_cfg,
                            &i2s_tx_dma_desc[(i + 1) % I2S_FRAME_NUM],
                            (i + 1) % (I2S_FRAME_NUM / 2) == 0);
    }

    hal_audma_sg_start(&i2s_tx_dma_desc[0], &dma_cfg);

    // hal_i2s_start_stream(i2s_id, AUD_STREAM_PLAYBACK);

    eshell_putstring("I2S: Playback Configuration done\r\n");
}

static void test_i2s_capture(enum HAL_I2S_ID_T i2s_id, enum AUD_BITS_T i2s_bit,
                             uint32_t sample_rate, enum HAL_DMA_WIDTH_T dma_width,
                             uint16_t dma_tsize)
{
    hal_i2s_open(i2s_id, AUD_STREAM_CAPTURE, HAL_I2S_MODE_MASTER);

    struct HAL_I2S_CONFIG_T i2s_cfg;
    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.use_dma = true;
    i2s_cfg.sync_start = false;
    i2s_cfg.chan_sep_buf = false;
    i2s_cfg.bits = i2s_bit;
    i2s_cfg.channel_num = 2;
    i2s_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    i2s_cfg.sample_rate = sample_rate;
    // i2s_cfg.ext_mclk_freq = 24576000;

    hal_i2s_setup_stream(i2s_id, AUD_STREAM_CAPTURE, &i2s_cfg);

    struct HAL_DMA_CH_CFG_T dma_cfg;
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_width = dma_width;
    dma_cfg.handler = i2s_rx_handler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_I2S0_RX;
    dma_cfg.src_tsize = dma_tsize;
    dma_cfg.src_width = dma_width;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (int i = 0; i < I2S_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&i2s_rx_buf[i][0];
        hal_audma_init_desc(&i2s_rx_dma_desc[i], &dma_cfg,
                            &i2s_rx_dma_desc[(i + 1) % I2S_FRAME_NUM],
                            (i + 1) % (I2S_FRAME_NUM / 2) == 0);
    }

    hal_audma_sg_start(&i2s_rx_dma_desc[0], &dma_cfg);

    // hal_i2s_start_stream(i2s_id, AUD_STREAM_CAPTURE);

    eshell_putstring("I2S: Capture Configuration done\r\n");
}

static void unitest_i2s(int argc, char *argv[])
{
    if (argc < 4) {
        eshell_putstring("cmd err\r\n");
        return;
    }
    enum HAL_I2S_ID_T i2s_id = atoi(argv[1]);
    uint32_t sample_rate = atoi(argv[3]);
    uint8_t i2s_bits = atoi(argv[4]);
    if (strncmp(argv[2], "loop_back", 9) == 0)
    {
        i2s_loop_test(i2s_id, sample_rate, i2s_bits);
        return;
    }

    if (argc < 7) {
        eshell_putstring("cmd err\r\n");
        eshell_putstring("cmd format: utest_i2s bus(0/1) item(play_only/play_capt) i2s_bit i2s_samprate dma_width(1/2/4) dma_tsize\r\n");
        return;
    }

    if ((i2s_id >= 0) && (i2s_id < 2)) {
        if (strncmp(argv[2], "play_only", 9) == 0) {
            enum AUD_BITS_T i2s_bit = atoi(argv[3]);
            uint32_t sample_rate = atoi(argv[4]);
            enum HAL_DMA_WIDTH_T dma_width = atoi(argv[5]) / 2;
            uint16_t dma_tsize = atoi(argv[6]);
            eshell_putstring("i2s_bit=%d i2s_samprate=%d width=%d tsize=%d\r\n", i2s_bit,
                             sample_rate, dma_width, dma_tsize);
            test_i2s_playback(i2s_id, i2s_bit, sample_rate, dma_width, dma_tsize);
            i2s_start_stream(i2s_id, AUD_STREAM_PLAYBACK);
            return;
        } else if (strncmp(argv[2], "play_capt", 9) == 0) {
            enum AUD_BITS_T i2s_bit = atoi(argv[3]);
            uint32_t sample_rate = atoi(argv[4]);
            enum HAL_DMA_WIDTH_T dma_width = atoi(argv[5]) / 2;
            uint16_t dma_tsize = atoi(argv[6]);
            eshell_putstring("i2s_bit=%d i2s_samprate=%d width=%d tsize=%d\r\n", i2s_bit,
                             sample_rate, dma_width, dma_tsize);
            test_i2s_capture(i2s_id, i2s_bit, sample_rate, dma_width, dma_tsize);
            test_i2s_playback(i2s_id, i2s_bit, sample_rate, dma_width, dma_tsize);
            i2s_start_stream(i2s_id, AUD_STREAM_PLAYBACK);
            i2s_start_stream(i2s_id, AUD_STREAM_CAPTURE);
            return;
        } else {
            eshell_putstring("invalid test_item, rang: play_only, play_capt\r\n");
            return;
        }
    } else {
        eshell_putstring("invalid i2s_id, rang:0,1\r\n");
        return;
    }
}

ESHELL_DEF_COMMAND(
    ESHELL_CMD_GRP_UNIT_TEST, "utest_i2s",
    "utest_i2s bus(0/1) item(play_only/play_capt) i2s_bit i2s_samprate dma_width(1/2/4) dma_tsize",
    unitest_i2s);

#endif // __SYS_AS_MAIN__ || CHIP_SUBSYS_SENS
#endif // UTILS_ESHELL_EN
