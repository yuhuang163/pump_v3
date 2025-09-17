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
#include "reg_dma.h"
#include "string.h"

#ifdef CHIP_BEST1000

// FM_SIMU
#define FM_FRAME_SIZE 16//512
#define FM_FRAME_NUM 32//136
#define FM_BUF_ALIGN __attribute__((aligned(0x100)))

static uint32_t FM_BUF_ALIGN fm_buf[FM_FRAME_NUM][FM_FRAME_SIZE / 4];
static struct HAL_DMA_DESC_T fm_dma_desc[FM_FRAME_NUM];
struct HAL_DMA_CH_CFG_T dma_cfg;
static struct DMA_T * const dma = (struct DMA_T *)AUDMA_BASE;

static void fm_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    if (++cnt >= 2) {
        // Stop FM
        REG(0xd0330038) = 0;
        // Make DMA consider single transfer request
        dma->CH[dma_cfg.ch].CONFIG &= ~DMA_CONFIG_TRY_BURST;
    }
    TRACE(3,"FM: remains=%d, error=%d, cnt=%d", remains, error, cnt);
}

void fm_test(void)
{
    int i;

    // FM simulation
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = fm_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_DPD_RX;
    dma_cfg.src_tsize = sizeof(fm_buf[0]) / 4;
    dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < FM_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&fm_buf[i][0];
        hal_audma_init_desc(&fm_dma_desc[i], &dma_cfg, &fm_dma_desc[(i + 1) % FM_FRAME_NUM], (i + 1) % (FM_FRAME_NUM / 2) == 0);
    }
    hal_audma_sg_start(&fm_dma_desc[0], &dma_cfg);

    REG(0xd0330038) |= (1 << 11);
    REG(DPD_RX_BASE + 0x30) = 1;
    REG(DPD_RX_BASE) = 1;
}

#endif // CHIP_BEST1000
