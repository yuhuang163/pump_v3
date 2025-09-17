/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
 *`
 ****************************************************************************/
#include "plat_addr_map.h"

#ifdef QSPI_BASE
#include <string.h>
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_qspi.h"
#include "reg_qspi.h"
#include "hal_iomux.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"

#ifdef QSPI_DEBUG
#define HAL_QSPI_TRACE(attr, str, ...)      TRACE(attr, str, ##__VA_ARGS__)
#define HAL_QSPI_ERROR(attr, str, ...)      TRACE(attr, str, ##__VA_ARGS__)
#else
#define HAL_QSPI_TRACE(attr, str, ...)      TRACE_DUMMY(attr, str, ##__VA_ARGS__)
#define HAL_QSPI_ERROR(attr, str, ...)      TRACE(attr, str, ##__VA_ARGS__)
#endif

#define QSPI_START                          QSPI_SSIENR_SSIC_EN
#define QSPI_CLOSE                          0
#define QSPI_TRANS_FIFO_THRESHOLD           (4*6)
#define QSPI_TRANS_DATA_LEVEL_TX            (0xF)
#define QSPI_TRANS_DATA_LEVEL_RX            (0x700)
#define QSPI_INTRUCTION_LEN                 HAL_QSPI_INST_L8

static struct QSPI_T * const qspi = (struct QSPI_T *)QSPI_BASE;

static struct HAL_QSPI_CTRL_T qspi_ctrl[HAL_QSPI_CS_QTY];

static enum HAL_QSPI_CS_T qspi0_cs = HAL_QSPI_CS_0;

static uint8_t qspi_txdma_chan;

static HAL_QSPI_DMA_HANDLER_T qspi_txdma_handler;

static enum HAL_QSPI_MOD_CLK_SEL_T qspi_clk;

static int hal_qspi_init_ctrl(const struct HAL_QSPI_CFG_T *cfg, struct HAL_QSPI_CTRL_T *ctrl)
{
    uint8_t POSSIBLY_UNUSED div, min_div = 2;
    uint32_t data_frame_size = 0, data_frame_number = 0, mod_freq = 0;

    if (cfg->slave == true) {
        ctrl->ser = QSPI_SER(0x0);
    } else {
        ctrl->ser = QSPI_SER(0x1);
    }

    if (cfg->mode >= HAL_QSPI_MODE_QTY || QSPI_INTRUCTION_LEN >= HAL_QSPI_INST_LEN_QTY) {
        return -1;
    }

#if defined(CHIP_BEST1700) || defined(CHIP_BEST2007)
    mod_freq = hal_cmu_sys_get_freq_value() / 2;
#else
#ifdef PERIPH_PLL_FREQ
    if (PERIPH_PLL_FREQ / 2 > 2 * hal_cmu_get_crystal_freq()) {
        // Init to pll
        mod_freq = 2 * hal_cmu_get_crystal_freq();
        if (cfg->rate * min_div > mod_freq) {
            mod_freq = PERIPH_PLL_FREQ / 2;
            ctrl->clk_sel = HAL_QSPI_MOD_CLK_SEL_PLL;
        } else {
            mod_freq = 0;
        }
    }
#else
    if (mod_freq == 0) {
        // Init to OSC
        mod_freq = hal_cmu_get_crystal_freq();
        if (cfg->rate * min_div > mod_freq) {
            mod_freq *= 2;
            ctrl->clk_sel = HAL_QSPI_MOD_CLK_SEL_OSC_X2;
        } else {
            ctrl->clk_sel = HAL_QSPI_MOD_CLK_SEL_OSC;
        }
    }
#endif
#endif
    div = (mod_freq + cfg->rate - 1) / cfg->rate;
    if (div < 1) {
        return -2;
    }

    data_frame_size = cfg->tx_bits - 1;
    if (cfg->mode == HAL_QSPI_STANDARD_MODE) {
        data_frame_number = 0x6;
    } else if (cfg->mode == HAL_QSPI_QUAD_MODE) {
        data_frame_number = 0xC800;
    }

    ctrl->ctrlr0 = QSPI_SCTRLR0_SPI_FRF(cfg->mode) |
        QSPI_SCTRLR0_TMOD(cfg->transfer_mode) |
        QSPI_SCTRLR0_DFS(data_frame_size);
    ctrl->ctrlr1 = QSPI_SCTRLR1_NDF(data_frame_number);
    ctrl->baudr = QSPI_BAUDR_SCKDV(div);

    if (cfg->mode == HAL_QSPI_QUAD_MODE) {
        ctrl->txftlr = QSPI_TRANS_FIFO_THRESHOLD;
        ctrl->spi_ctrlr0 = QSPI_SPI_CTRLR0_CLK_STRETCH_EN |
            QSPI_SPI_CTRLR0_INST_L(QSPI_INTRUCTION_LEN);
    }

    ctrl->dmacr = (cfg->dma_tx ? QSPI_SPI_DMACR_TDMAE : 0) |
        (cfg->dma_rx ? QSPI_SPI_DMACR_RDMAE : 0);
    if (cfg->dma_tx) {
        ctrl->dmatdlr = QSPI_DMATDLR(QSPI_TRANS_DATA_LEVEL_TX);
    }

    return 0;
}

static void hal_qspi_enable(const struct HAL_QSPI_CTRL_T *ctrl)
{
    qspi->SSIENR = QSPI_CLOSE;

    qspi->SCTRLR0 = ctrl->ctrlr0;
    qspi->SCTRLR1 = ctrl->ctrlr1;
    qspi->SER = ctrl->ser;
    qspi->BAUDR = ctrl->baudr;
    if (ctrl->ctrlr0 & QSPI_SCTRLR0_SPI_FRF(0x2)) {
        qspi->TXFTLR = ctrl->txftlr;
        qspi->SPI_CTRLR0 = ctrl->spi_ctrlr0;
    }
    qspi->DMACR = ctrl->dmacr;
    if (ctrl->dmacr & QSPI_SPI_DMACR_TDMAE || ctrl->dmacr & QSPI_SPI_DMACR_IDMAE) {
        qspi->DMATDLR = ctrl->dmatdlr;
    }

    if (qspi_clk != ctrl->clk_sel) {
        qspi_clk = ctrl->clk_sel;
        if (0) {
#ifdef PERIPH_PLL_FREQ
        } else if (ctrl->clk_sel == HAL_QSPI_MOD_CLK_SEL_PLL) {
            hal_cmu_qspi_set_div(2);
#endif
        } else {
            enum HAL_CMU_PERIPH_FREQ_T POSSIBLY_UNUSED periph_freq;

            if (ctrl->clk_sel == HAL_QSPI_MOD_CLK_SEL_OSC_X2) {
                periph_freq = HAL_CMU_PERIPH_FREQ_52M;
            } else {
                periph_freq = HAL_CMU_PERIPH_FREQ_26M;
            }
            hal_cmu_qspi_set_freq(periph_freq);
        }
    }

    qspi->SSIENR = QSPI_START;
}

int hal_qspi_open(struct HAL_QSPI_CFG_T *cfg)
{
    int ret, lock;

    if (cfg->cs >= HAL_QSPI_CS_QTY) {
        return 1;
    }
    qspi0_cs = cfg->cs;

    hal_cmu_clock_enable(HAL_CMU_MOD_H_QSPI);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_QSPI);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_QSPI);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_QSPI);

    ret = hal_qspi_init_ctrl(cfg, &qspi_ctrl[qspi0_cs]);
    if (ret) {
        return ret;
    }

    HAL_QSPI_TRACE(0, " ");
    HAL_QSPI_TRACE(0, "********************************");
    HAL_QSPI_TRACE(0, "****ctrl->ctrlr0 = 0x%x", qspi_ctrl[qspi0_cs].ctrlr0);
    HAL_QSPI_TRACE(0, "****ctrl->ctrlr1 = 0x%x", qspi_ctrl[qspi0_cs].ctrlr1);
    HAL_QSPI_TRACE(0, "****ctrl->ser    = 0x%x", qspi_ctrl[qspi0_cs].ser);
    HAL_QSPI_TRACE(0, "****ctrl->baudr  = 0x%x", qspi_ctrl[qspi0_cs].baudr);
    if (cfg->mode == HAL_QSPI_QUAD_MODE) {
        HAL_QSPI_TRACE(0, "****ctrl->txftlr = 0x%x", qspi_ctrl[qspi0_cs].txftlr);
        HAL_QSPI_TRACE(0, "****ctrl->spi_ctrlr0 = 0x%x", qspi_ctrl[qspi0_cs].spi_ctrlr0);
    }
    if (cfg->dma_tx == true || cfg->dma_rx == true) {
        HAL_QSPI_TRACE(0, "****ctrl->dmacr   = 0x%x", qspi_ctrl[qspi0_cs].dmacr);
        HAL_QSPI_TRACE(0, "****ctrl->dmatdlr = 0x%x", qspi_ctrl[qspi0_cs].dmatdlr);
    }
    HAL_QSPI_TRACE(0, "********************************\n");

    lock = int_lock();
    hal_qspi_enable(&qspi_ctrl[qspi0_cs]);
    int_unlock(lock);

    return 0;
}

POSSIBLY_UNUSED
static void hal_qspi_rw_ctrl_switch(enum HAL_QSPI_TRANSFER_MODE_T trans_mode)
{
    int lock;

    if (((qspi_ctrl[qspi0_cs].ctrlr0 >> QSPI_SCTRLR0_SPI_FRF_SHIFT) & 0x3) == HAL_QSPI_STANDARD_MODE ||
        ((qspi_ctrl[qspi0_cs].ctrlr0 >> QSPI_SCTRLR0_TMOD_SHIFT) & 0x3) == trans_mode) {
        return;
    }

    switch (trans_mode) {
    case HAL_QSPI_RX_ONLY:
        qspi_ctrl[qspi0_cs].ctrlr0 &= ~QSPI_SCTRLR0_TMOD_MASK;
        qspi_ctrl[qspi0_cs].ctrlr0 |= QSPI_SCTRLR0_TMOD(HAL_QSPI_RX_ONLY);
        qspi_ctrl[qspi0_cs].ctrlr1 = QSPI_SCTRLR1_NDF(0x383ff);
        qspi_ctrl[qspi0_cs].dmacr = QSPI_SPI_DMACR_AINC |
            QSPI_SPI_DMACR_ATW(0x3) | QSPI_SPI_DMACR_IDMAE;
        qspi_ctrl[qspi0_cs].dmatdlr = QSPI_DMATDLR(QSPI_TRANS_DATA_LEVEL_RX);
        break;

    case HAL_QSPI_TX_ONLY:
        qspi_ctrl[qspi0_cs].ctrlr0 &= ~QSPI_SCTRLR0_TMOD_MASK;
        qspi_ctrl[qspi0_cs].ctrlr0 |= QSPI_SCTRLR0_TMOD(HAL_QSPI_TX_ONLY);
        qspi_ctrl[qspi0_cs].ctrlr1 = QSPI_SCTRLR1_NDF(0xc800);
        qspi_ctrl[qspi0_cs].dmacr = QSPI_SPI_DMACR_TDMAE;
        qspi_ctrl[qspi0_cs].dmatdlr = QSPI_DMATDLR(QSPI_TRANS_DATA_LEVEL_TX);
        break;

    default:
        break;
    }

    lock = int_lock();
    hal_qspi_enable(&qspi_ctrl[qspi0_cs]);
    int_unlock(lock);
}

static uint8_t get_frame_bytes(uint32_t ctrlr0)
{
    uint16_t bits;
    uint8_t cnt;

    bits = GET_BITFIELD(ctrlr0, QSPI_SCTRLR0_DFS) + 1;
    if (bits <= 8) {
        cnt = 1;
    } else if (bits <= 16) {
        cnt = 2;
    } else{
        cnt = 4;
    }

    return cnt;
}

static void copy_frame_from_bytes(uint32_t *val, const uint8_t *data, uint8_t cnt)
{
    if (cnt == 1) {
        *val = data[0];
    } else if (cnt == 2) {
        *val = data[0] | (data[1] << 8);
    } else {
        *val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    }
}

int hal_qspi_send(const void *data, uint32_t len)
{
    uint32_t send, val;
    uint8_t cnt;
    int ret;

    hal_qspi_rw_ctrl_switch(HAL_QSPI_TX_ONLY);

    cnt = get_frame_bytes(qspi->SCTRLR0);
    if (len == 0 || (len & (cnt - 1)) != 0) {
        return -1;
    }

    send = 0;
    while (send < len) {
        if (!(qspi->SSIENR & QSPI_SSIENR_SSIC_EN)) {
            break;
        }
        val = 0;
        copy_frame_from_bytes(&val, (uint8_t *)data + send, cnt);
        qspi->DR = val;
        send += cnt;
    }

    if (send >= len) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

static void hal_qspi_txdma_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    if (chan != qspi_txdma_chan) {
        return;
    }

    qspi_txdma_chan = HAL_DMA_CHAN_NONE;
    hal_gpdma_free_chan(chan);

    if (qspi_txdma_handler) {
        qspi_txdma_handler(error);
    }
}

int hal_qspi_dma_send(const void *data, uint32_t len, HAL_QSPI_DMA_HANDLER_T handler)
{
    uint8_t cnt;
    int ret = 0;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_WIDTH_T dma_width;
    enum HAL_DMA_PERIPH_T dst_periph = HAL_DMA_PERIPH_NULL;

    hal_qspi_rw_ctrl_switch(HAL_QSPI_TX_ONLY);
    ASSERT((qspi->DMACR & QSPI_SPI_DMACR_TDMAE), "TX-DMA not configured on QSPI");

    cnt = get_frame_bytes(qspi->SCTRLR0);
    if (len == 0 || (len & (cnt - 1)) != 0) {
        return -1;
    }
    if (((uint32_t)data & (cnt - 1)) != 0) {
        return -2;
    }

    if (cnt == 1) {
        dma_width = HAL_DMA_WIDTH_BYTE;
    } else if (cnt == 2) {
        dma_width = HAL_DMA_WIDTH_HALFWORD;
    } else {
        dma_width = HAL_DMA_WIDTH_WORD;
    }

    qspi_txdma_handler = handler;

    dst_periph = HAL_GPDMA_QSPI_TX;
    qspi_txdma_chan = hal_gpdma_get_chan(dst_periph, HAL_DMA_HIGH_PRIO);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.ch = qspi_txdma_chan;
    dma_cfg.dst = 0;
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_1;
    dma_cfg.dst_periph = dst_periph;
    dma_cfg.dst_width = dma_width;
    dma_cfg.handler = handler ? hal_qspi_txdma_handler : NULL;
    dma_cfg.src = (uint32_t)data;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_1;
    dma_cfg.src_tsize = len / cnt;
    dma_cfg.src_width = dma_width;
    dma_cfg.try_burst = 0;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;

    ret = hal_gpdma_start(&dma_cfg);
    if (ret != HAL_DMA_OK) {
        return -3;
    }

    if (handler == NULL) {
        while (hal_gpdma_chan_busy(qspi_txdma_chan));
        hal_gpdma_free_chan(qspi_txdma_chan);
        qspi_txdma_chan = HAL_DMA_CHAN_NONE;
    }

    return ret;
}

#endif

