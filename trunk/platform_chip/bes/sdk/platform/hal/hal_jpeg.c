/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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

#include "cmsis.h"
#include "plat_addr_map.h"

#ifdef JPEG_BASE
#include "string.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_chipid.h"
#include "reg_jpeg.h"
#include "hal_jpeg.h"


//#define JPEG_DEBUG
#ifdef  JPEG_DEBUG
#define JPEG_INFO(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define JPEG_INFO(...)
#endif

#define JPEG_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

static struct JPEG_REG_T *jpeg_base[] = {
    (struct JPEG_REG_T *)JPEG_BASE,
#ifdef JPEG1_BASE
    (struct JPEG_REG_T *)JPEG1_BASE,
#endif
};

/* For 1503 old version chips, metal id < 6 */
#ifdef CHIP_BEST1503
#define JPEG_HSYMB_MEM_V0       0x4000
#define JPEG_HMIN_MEM_V0        0x5000
#define JPEG_HBASE_MEM_V0       0x6000
#define JPEG_QTABLE_MEM_V0      0x7000
#endif

#define JPEG_HSYMB_MEM          0x7000
#define JPEG_HMIN_MEM           0x8000
#define JPEG_HBASE_MEM          0x9000
#define JPEG_QTABLE_MEM         0xa000
#define JPEG_HETAB_MEM          0xb000

#define JPEG_ECS_BUF            0x3000
#define JPEG_PIX_BUF            0xC000

#if defined(CHIP_BEST1503) || defined(CHIP_BEST1700) || defined(CHIP_BEST1306P)
#define JPEG_DEC_BLK_SZ         (0xf0000)
#else
#define JPEG_DEC_BLK_SZ         (0xffffffff)
#endif

#define DMA_XFER_BLK_SZ         (0xfa0*4)
#define DMA_ECS_MAX_CNT         (32)
#define DMA_PIX_MAX_CNT         (128)

static enum HAL_DMA_BSIZE_T jpeg_xfer_bsize = HAL_DMA_BSIZE_16;

static ALIGNED(16) struct HAL_DMA_DESC_T pix_desc_buf[DMA_PIX_MAX_CNT * HAL_JPEG_ID_QTY];
static ALIGNED(16) struct HAL_DMA_DESC_T ecs_desc_buf[DMA_ECS_MAX_CNT * HAL_JPEG_ID_QTY];

struct JPEG_CODEC_T {
    struct HAL_JPEG_CONFIG_T *cfg;

    bool opened;
    bool working;

    int pix_ch;
    int ecs_ch;
    struct HAL_DMA_DESC_T *pix_desc;
    struct HAL_DMA_DESC_T *ecs_desc;

    uint32_t dec_size;
};

struct JPEG_CODEC_T jpeg_codec[HAL_JPEG_ID_QTY];

static void jpeg0_pix_rx_handler(uint8_t chan, uint32_t remain_tsize,
        uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    struct JPEG_CODEC_T *codec = &jpeg_codec[HAL_JPEG_ID_0];

    if (error)
        JPEG_ERR("JPEG[0] pixel data decode failed!");
    else
        JPEG_INFO("JPEG[0] pixel data decode done!");

    if (codec->cfg && codec->cfg->done_cb)
        codec->cfg->done_cb(HAL_JPEG_ID_0);

    /* Decode is done, release DMA channels. */
    if (codec->pix_ch >= 0) {
        hal_dma_free_chan(codec->pix_ch);
        codec->pix_ch = -1;
    }
    if (codec->ecs_ch >= 0) {
        hal_dma_free_chan(codec->ecs_ch);
        codec->ecs_ch = -1;
    }

    codec->working = false;
}

#ifdef JPEG1_BASE
static void jpeg1_pix_rx_handler(uint8_t chan, uint32_t remain_tsize,
        uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    struct JPEG_CODEC_T *codec = &jpeg_codec[HAL_JPEG_ID_1];

    if (error)
        JPEG_ERR("JPEG[1] pixel data decode failed!");
    else
        JPEG_INFO("JPEG[1] pixel data decode done!");

    if (codec->cfg && codec->cfg->done_cb)
        codec->cfg->done_cb(HAL_JPEG_ID_1);

    /* Decode is done, release DMA channels. */
    if (codec->pix_ch >= 0) {
        hal_dma_free_chan(codec->pix_ch);
        codec->pix_ch = -1;
    }
    if (codec->ecs_ch >= 0) {
        hal_dma_free_chan(codec->ecs_ch);
        codec->ecs_ch = -1;
    }

    codec->working = false;
}
#endif

/* Pixel data receive for decode */
static int hal_jpeg_pix_rx(enum HAL_JPEG_ID_T id, void *rx_buf, uint32_t rx_size)
{
    struct JPEG_CODEC_T *codec = &jpeg_codec[id];
    uint32_t total_sz = 0;
    int block_sz = DMA_XFER_BLK_SZ;
    int desc_count = 0;
    enum HAL_DMA_RET_T ret = HAL_DMA_OK;
    struct HAL_DMA_DESC_T *desc = codec->pix_desc;
    struct HAL_DMA_CH_CFG_T cfg;

    /* DMA xfer size is aligned to 4 bytes */
    if (rx_size % 4)
        rx_size += 4 - (rx_size % 4);

    if (rx_size % DMA_XFER_BLK_SZ)
        desc_count = rx_size / block_sz + 1;
    else
        desc_count = rx_size / block_sz;

    if (desc_count > DMA_PIX_MAX_CNT)
        return HAL_JPEG_PIX_DMA_ERR;

    memset(&cfg, 0, sizeof(cfg));
    cfg.type = HAL_DMA_FLOW_P2M_DMA | HAL_DMA_FLOW_FLAG_SI;
#ifdef CHIP_BEST1503
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        cfg.type = HAL_DMA_FLOW_P2M_DMA;
#endif
    cfg.src = (uint32_t)jpeg_base[id] + JPEG_PIX_BUF;
    cfg.src_tsize = block_sz/4;
    cfg.src_bsize = jpeg_xfer_bsize;
    cfg.src_width = HAL_DMA_WIDTH_WORD;
    cfg.src_periph = HAL_GPDMA_JPEG_PIX;

    cfg.dst = ADDR_CPU_TO_DEV((uint32_t)rx_buf);
    cfg.dst_bsize = jpeg_xfer_bsize;
    cfg.dst_width = HAL_DMA_WIDTH_WORD;
    cfg.dst_periph = HAL_DMA_PERIPH_NULL;

    cfg.try_burst = 0;
    cfg.handler = jpeg0_pix_rx_handler;
    cfg.start_cb = NULL;

#ifdef JPEG1_BASE
    if (id == HAL_JPEG_ID_1) {
        cfg.src_periph = HAL_GPDMA_JPEG1_PIX;
        cfg.handler = jpeg1_pix_rx_handler;
    }
#endif

    cfg.ch = hal_dma_get_chan(HAL_GPDMA_MEM, HAL_DMA_HIGH_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        JPEG_ERR("JPEG[%d] ERROR: failed to get PIX DMA channel.", id);
        return HAL_JPEG_PIX_DMA_ERR;
    }
    codec->pix_ch = cfg.ch;

    for (int i = 0; i < desc_count; i++) {
        uint32_t sz = (total_sz + block_sz <= rx_size) ? block_sz : (rx_size - total_sz);
        cfg.src_tsize = sz / 4;
        if (i+1 == desc_count)
            hal_dma_init_desc(&desc[i], &cfg, NULL, 1);
        else
            hal_dma_init_desc(&desc[i], &cfg, &desc[i+1], 0);
        JPEG_INFO("JPEG[%d] PIX DMA Desc[%d]: %x, %x, %x, %x", id, i,
                desc[i].src, desc[i].dst, desc[i].lli, desc[i].ctrl);

        total_sz += sz;
        cfg.dst += sz;
    }

    ret = hal_dma_sg_start(&desc[0], &cfg);
    if (ret) {
        JPEG_ERR("JPEG[%d] ERROR: faild to start PIX transfer.", id);
        return HAL_JPEG_PIX_DMA_ERR;
    }
    return HAL_JPEG_OK;
}

/* ECS data send for decode */
static int hal_jpeg_ecs_tx(enum HAL_JPEG_ID_T id, void *tx_buf, uint32_t tx_size)
{
    struct JPEG_CODEC_T *codec = &jpeg_codec[id];
    enum HAL_DMA_RET_T ret = HAL_DMA_OK;
    int total_sz = 0;
    int block_sz = DMA_XFER_BLK_SZ;
    int desc_count = 0;
    struct HAL_DMA_DESC_T *desc = codec->ecs_desc;
    struct HAL_DMA_CH_CFG_T cfg;

    /* DMA xfer size is aligned to 4 bytes */
    if (tx_size % 4)
        tx_size += 4 - (tx_size % 4);

    if (tx_size % DMA_XFER_BLK_SZ)
        desc_count = tx_size / block_sz + 1;
    else
        desc_count = tx_size / block_sz;

    if (desc_count > DMA_ECS_MAX_CNT)
        return HAL_JPEG_ECS_DMA_ERR;

    memset(&cfg, 0, sizeof(cfg));
    cfg.type = HAL_DMA_FLOW_M2P_DMA;

    cfg.src = ADDR_CPU_TO_DEV((uint32_t)tx_buf);
    cfg.src_tsize = block_sz / 4;
    cfg.src_bsize = jpeg_xfer_bsize;
    cfg.src_width = HAL_DMA_WIDTH_WORD;
    cfg.src_periph = HAL_DMA_PERIPH_NULL;

    cfg.dst = (uint32_t)jpeg_base[id] + JPEG_ECS_BUF;
    cfg.dst_bsize = jpeg_xfer_bsize;
    cfg.dst_width = HAL_DMA_WIDTH_WORD;
    cfg.dst_periph = HAL_GPDMA_JPEG_ECS;

    cfg.try_burst = 0;
    cfg.handler = NULL;
    cfg.start_cb = NULL;

#ifdef JPEG1_BASE
    if (id == HAL_JPEG_ID_1) {
        cfg.dst_periph = HAL_GPDMA_JPEG1_ECS;
    }
#endif

    cfg.ch = hal_dma_get_chan(cfg.dst_periph, HAL_DMA_LOW_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        JPEG_ERR("JPEG[%d] ERROR: failed to get ECS DMA channel.", id);
        return HAL_JPEG_ECS_DMA_ERR;
    }
    codec->ecs_ch = cfg.ch;

    for (int i = 0; i < desc_count; i++) {
        uint32_t sz = (total_sz + block_sz <= tx_size) ? block_sz : (tx_size - total_sz);
        cfg.src_tsize = sz / 4;
        if (i+1 == desc_count)
            hal_dma_init_desc(&desc[i], &cfg, NULL, 0);
        else
            hal_dma_init_desc(&desc[i], &cfg, &desc[i+1], 0);
        JPEG_INFO("JPEG[%d] ECS DMA Desc[%d]: %x, %x, %x, %x", id, i,
                desc[i].src, desc[i].dst, desc[i].lli, desc[i].ctrl);

        total_sz += sz;
        cfg.src += sz;
    }

    ret = hal_dma_sg_start(&desc[0], &cfg);
    if (ret) {
        JPEG_ERR("JPEG[%d] ERROR: failed to start ECS transfer.", id);
        return HAL_JPEG_ECS_DMA_ERR;
    }
    return HAL_JPEG_OK;
}

/* Pixel data send for encode */
static int hal_jpeg_pix_tx(enum HAL_JPEG_ID_T id, void *tx_buf, uint32_t tx_size)
{
    struct JPEG_CODEC_T *codec = &jpeg_codec[id];
    enum HAL_DMA_RET_T ret = HAL_DMA_OK;
    int total_sz = 0;
    int block_sz = DMA_XFER_BLK_SZ;
    int desc_count = 0;
    struct HAL_DMA_DESC_T *desc = codec->pix_desc;
    struct HAL_DMA_CH_CFG_T cfg;

    /* DMA xfer size is aligned to 4 bytes */
    if (tx_size % 4)
        tx_size += 4 - (tx_size % 4);

    if (tx_size % DMA_XFER_BLK_SZ)
        desc_count = tx_size / block_sz + 1;
    else
        desc_count = tx_size / block_sz;

    if (desc_count > DMA_PIX_MAX_CNT)
        return HAL_JPEG_PIX_DMA_ERR;

    memset(&cfg, 0, sizeof(cfg));
    cfg.type = HAL_DMA_FLOW_M2P_DMA | HAL_DMA_FLOW_FLAG_DI;

    cfg.src = ADDR_CPU_TO_DEV((uint32_t)tx_buf);
    cfg.src_tsize = block_sz / 4;
    cfg.src_bsize = jpeg_xfer_bsize;
    cfg.src_width = HAL_DMA_WIDTH_WORD;
    cfg.src_periph = HAL_DMA_PERIPH_NULL;

    cfg.dst = (uint32_t)jpeg_base[id] + JPEG_PIX_BUF;
    cfg.dst_bsize = jpeg_xfer_bsize;
    cfg.dst_width = HAL_DMA_WIDTH_WORD;
    cfg.dst_periph = HAL_GPDMA_JPEG_PIX;

    cfg.try_burst = 0;
    cfg.handler = NULL;
    cfg.start_cb = NULL;

#ifdef JPEG1_BASE
    if (id == HAL_JPEG_ID_1) {
        cfg.dst_periph = HAL_GPDMA_JPEG1_PIX;
    }
#endif

    cfg.ch = hal_dma_get_chan(cfg.dst_periph, HAL_DMA_LOW_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        JPEG_ERR("JPEG[%d] ERROR: failed to get PIX DMA channel.", id);
        return HAL_JPEG_PIX_DMA_ERR;
    }
    codec->ecs_ch = cfg.ch;

    for (int i = 0; i < desc_count; i++) {
        uint32_t sz = (total_sz + block_sz <= tx_size) ? block_sz : (tx_size - total_sz);
        cfg.src_tsize = sz / 4;
        if (i+1 == desc_count)
            hal_dma_init_desc(&desc[i], &cfg, NULL, 0);
        else
            hal_dma_init_desc(&desc[i], &cfg, &desc[i+1], 0);
        JPEG_INFO("JPEG[%d] PIX DMA Desc[%d]: %x, %x, %x, %x", id, i,
                desc[i].src, desc[i].dst, desc[i].lli, desc[i].ctrl);

        total_sz += sz;
        cfg.src += sz;
    }

    ret = hal_dma_sg_start(&desc[0], &cfg);
    if (ret) {
        JPEG_ERR("JPEG[%d] ERROR: failed to start PIX transfer.", id);
        return HAL_JPEG_PIX_DMA_ERR;
    }
    return HAL_JPEG_OK;
}

/* ECS data receive for encode */
static int hal_jpeg_ecs_rx(enum HAL_JPEG_ID_T id, void *rx_buf, uint32_t rx_size)
{
    struct JPEG_CODEC_T *codec = &jpeg_codec[id];
    uint32_t total_sz = 0;
    int block_sz = DMA_XFER_BLK_SZ;
    int desc_count = 0;
    enum HAL_DMA_RET_T ret = HAL_DMA_OK;
    struct HAL_DMA_DESC_T *desc = codec->ecs_desc;
    struct HAL_DMA_CH_CFG_T cfg;

    /* DMA xfer size is aligned to 4 bytes */
    if (rx_size % 4)
        rx_size += 4 - (rx_size % 4);

    if (rx_size % DMA_XFER_BLK_SZ)
        desc_count = rx_size / block_sz + 1;
    else
        desc_count = rx_size / block_sz;

    if (desc_count > DMA_ECS_MAX_CNT)
        return HAL_JPEG_ECS_DMA_ERR;

    memset(&cfg, 0, sizeof(cfg));
    cfg.type = HAL_DMA_FLOW_P2M_DMA;

    cfg.src = (uint32_t)jpeg_base[id] + JPEG_ECS_BUF;
    cfg.src_tsize = block_sz/4;
    cfg.src_bsize = jpeg_xfer_bsize;
    cfg.src_width = HAL_DMA_WIDTH_WORD;
    cfg.src_periph = HAL_GPDMA_JPEG_ECS;

    cfg.dst = ADDR_CPU_TO_DEV((uint32_t)rx_buf);
    cfg.dst_bsize = jpeg_xfer_bsize;
    cfg.dst_width = HAL_DMA_WIDTH_WORD;
    cfg.dst_periph = HAL_DMA_PERIPH_NULL;

    cfg.try_burst = 0;
    cfg.handler = NULL;
    cfg.start_cb = NULL;

#ifdef JPEG1_BASE
    if (id == HAL_JPEG_ID_1) {
        cfg.src_periph = HAL_GPDMA_JPEG1_ECS;
    }
#endif

    cfg.ch = hal_dma_get_chan(HAL_GPDMA_MEM, HAL_DMA_HIGH_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        JPEG_ERR("JPEG[%d] ERROR: failed to get ECS DMA channel.", id);
        return HAL_JPEG_ECS_DMA_ERR;
    }
    codec->pix_ch = cfg.ch;

    for (int i = 0; i < desc_count; i++) {
        uint32_t sz = (total_sz + block_sz <= rx_size) ? block_sz : (rx_size - total_sz);
        cfg.src_tsize = sz / 4;
        if (i+1 == desc_count)
            hal_dma_init_desc(&desc[i], &cfg, NULL, 1);
        else
            hal_dma_init_desc(&desc[i], &cfg, &desc[i+1], 0);
        JPEG_INFO("JPEG[%d] ECS DMA Desc[%d]: %x, %x, %x, %x", id, i,
                desc[i].src, desc[i].dst, desc[i].lli, desc[i].ctrl);

        total_sz += sz;
        cfg.dst += sz;
    }

    ret = hal_dma_sg_start(&desc[0], &cfg);
    if (ret) {
        JPEG_ERR("JPEG[%d] ERROR: faild to start ECS transfer.", id);
        return HAL_JPEG_ECS_DMA_ERR;
    }
    return HAL_JPEG_OK;
}

static void hal_jpeg_write_qtable(enum HAL_JPEG_ID_T id, uint8_t *data, uint32_t size)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_QTABLE_MEM);
    uint32_t *val = (uint32_t *)data;
#ifdef CHIP_BEST1503
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_QTABLE_MEM_V0);
#endif
    for (int i = 0; i < size/sizeof(uint32_t); i++)
        reg[i] = val[i];
}

static void hal_jpeg_write_hetable(enum HAL_JPEG_ID_T id, uint8_t *data, uint32_t size)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_HETAB_MEM);
    uint32_t *val = (uint32_t *)data;
    for (int i = 0; i < size/sizeof(uint32_t); i++)
        reg[i] = val[i];
}

static void hal_jpeg_write_hbase(enum HAL_JPEG_ID_T id, uint8_t *data, uint32_t size)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_HBASE_MEM);
    uint32_t *val = (uint32_t *)data;
#ifdef CHIP_BEST1503
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_HBASE_MEM_V0);
#endif
    for (int i = 0; i < size/sizeof(uint32_t); i++)
        reg[i] = val[i];
}

static void hal_jpeg_write_hmin(enum HAL_JPEG_ID_T id, uint8_t *data, uint32_t size)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_HMIN_MEM);
    uint32_t *val = (uint32_t *)data;
#ifdef CHIP_BEST1503
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_HMIN_MEM_V0);
#endif
    for (int i = 0; i < size/sizeof(uint32_t); i++)
        reg[i] = val[i];
}

static void hal_jpeg_write_hsymb(enum HAL_JPEG_ID_T id, uint8_t *data, uint32_t size)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_HSYMB_MEM);
    uint32_t *val = (uint32_t *)data;
#ifdef CHIP_BEST1503
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        reg = (volatile uint32_t *)((uint32_t)jpeg_base[id] + JPEG_HSYMB_MEM_V0);
#endif
    for (int i = 0; i < size/sizeof(uint32_t); i++)
        reg[i] = val[i];
}

int hal_jpeg_open(enum HAL_JPEG_ID_T id)
{
    if (id >= HAL_JPEG_ID_QTY)
        return HAL_JPEG_ERR;
    if (jpeg_codec[id].opened)
        return HAL_JPEG_OK;

#ifdef CHIP_BEST1503
    /* Workaround for best1503 old version */
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        jpeg_xfer_bsize = HAL_DMA_BSIZE_8;
#endif

    if (id == HAL_JPEG_ID_0)
        hal_cmu_jpeg_clock_enable();
#ifdef JPEG1_BASE
    else if (id == HAL_JPEG_ID_1)
        hal_cmu_jpeg1_clock_enable();
#endif

    jpeg_codec[id].cfg = NULL;
    jpeg_codec[id].pix_ch = -1;
    jpeg_codec[id].ecs_ch = -1;
    jpeg_codec[id].pix_desc = &pix_desc_buf[DMA_PIX_MAX_CNT * id];
    jpeg_codec[id].ecs_desc = &ecs_desc_buf[DMA_ECS_MAX_CNT * id];
    jpeg_codec[id].dec_size = 0;
    jpeg_codec[id].working = false;
    jpeg_codec[id].opened = true;
    return HAL_JPEG_OK;
}

int hal_jpeg_close(enum HAL_JPEG_ID_T id)
{
    if (id >= HAL_JPEG_ID_QTY)
        return HAL_JPEG_ERR;
    if (!jpeg_codec[id].opened)
        return HAL_JPEG_OK;

    if (id == HAL_JPEG_ID_0)
        hal_cmu_jpeg_clock_disable();
#ifdef JPEG1_BASE
    else if (id == HAL_JPEG_ID_1)
        hal_cmu_jpeg1_clock_disable();
#endif

    jpeg_codec[id].cfg = NULL;
    jpeg_codec[id].pix_ch = -1;
    jpeg_codec[id].ecs_ch = -1;
    jpeg_codec[id].dec_size = 0;
    jpeg_codec[id].working = false;
    jpeg_codec[id].opened = false;
    return HAL_JPEG_OK;
}

static void jpeg0_irq_handler(void)
{
    struct JPEG_REG_T *jpeg = jpeg_base[HAL_JPEG_ID_0];
    struct JPEG_CODEC_T *codec = &jpeg_codec[HAL_JPEG_ID_0];
    struct HAL_JPEG_CONFIG_T *cfg = codec->cfg;
    uint8_t val = jpeg->REG_40;
    JPEG_INFO("JPEG[0] IRQ: 0x%x", val);
    jpeg->REG_40 = val;

    if (!cfg)
        return;

    if (cfg->mode == HAL_JPEG_DEC_MODE &&
        val & JPEG_INTR_PIX_XFER_DONE) {
        if (codec->dec_size > 0) {
            /* continue pixel decoding */
            if (codec->dec_size > JPEG_DEC_BLK_SZ) {
                jpeg->REG_10 = JPEG_PIX_XFER_LEN(JPEG_DEC_BLK_SZ);
                codec->dec_size -= JPEG_DEC_BLK_SZ;
            } else {
                jpeg->REG_10 = JPEG_PIX_XFER_LEN(codec->dec_size);
                codec->dec_size = 0;
            }
            jpeg->REG_1000 = JPEG_CODEC_START;
            jpeg->REG_08 = JPEG_PIX_XFER_START;
        }
    } else if (cfg->mode == HAL_JPEG_ENC_MODE &&
               val & JPEG_INTR_ECS_XFER_DONE) {
        /* ecs transfer is done */

        /* Get the exact ecs size */
        cfg->ecs_size = jpeg->REG_1C;

        if (cfg->done_cb)
            cfg->done_cb(HAL_JPEG_ID_0);

        /* release DMA channels. */
        if (codec->pix_ch >= 0) {
            hal_dma_free_chan(codec->pix_ch);
            codec->pix_ch = -1;
        }
        if (codec->ecs_ch >= 0) {
            hal_dma_free_chan(codec->ecs_ch);
            codec->ecs_ch = -1;
        }

        codec->working = false;
    }
}

#ifdef JPEG1_BASE
static void jpeg1_irq_handler(void)
{
    struct JPEG_REG_T *jpeg = jpeg_base[HAL_JPEG_ID_1];
    struct JPEG_CODEC_T *codec = &jpeg_codec[HAL_JPEG_ID_1];
    struct HAL_JPEG_CONFIG_T *cfg = codec->cfg;
    uint8_t val = jpeg->REG_40;
    JPEG_INFO("JPEG[1] IRQ: 0x%x", val);
    jpeg->REG_40 = val;

    if (!cfg)
        return;

    if (cfg->mode == HAL_JPEG_DEC_MODE &&
        val & JPEG_INTR_PIX_XFER_DONE) {
        if (codec->dec_size > 0) {
            /* continue pixel decoding */
            if (codec->dec_size > JPEG_DEC_BLK_SZ) {
                jpeg->REG_10 = JPEG_PIX_XFER_LEN(JPEG_DEC_BLK_SZ);
                codec->dec_size -= JPEG_DEC_BLK_SZ;
            } else {
                jpeg->REG_10 = JPEG_PIX_XFER_LEN(codec->dec_size);
                codec->dec_size = 0;
            }
            jpeg->REG_1000 = JPEG_CODEC_START;
            jpeg->REG_08 = JPEG_PIX_XFER_START;
        }
    } else if (cfg->mode == HAL_JPEG_ENC_MODE &&
               val & JPEG_INTR_ECS_XFER_DONE) {
        /* ecs transfer is done */

        /* Get the exact ecs size */
        cfg->ecs_size = jpeg->REG_1C;

        if (cfg->done_cb)
            cfg->done_cb(HAL_JPEG_ID_1);

        /* release DMA channels. */
        if (codec->pix_ch >= 0) {
            hal_dma_free_chan(codec->pix_ch);
            codec->pix_ch = -1;
        }
        if (codec->ecs_ch >= 0) {
            hal_dma_free_chan(codec->ecs_ch);
            codec->ecs_ch = -1;
        }

        codec->working = false;
    }
}
#endif

static void hal_jpeg_irq_enable(enum HAL_JPEG_ID_T id, enum HAL_JPEG_MODE_T mode)
{
    struct JPEG_REG_T *jpeg = jpeg_base[id];

    if (mode == HAL_JPEG_ENC_MODE)
        jpeg->REG_44 = ~(JPEG_INTR_ECS_XFER_DONE_MASK);
    else
        jpeg->REG_44 = ~(JPEG_INTR_PIX_XFER_DONE_MASK);

    if (id == HAL_JPEG_ID_0) {
        NVIC_SetVector(JPEG_DEC_IRQn, (uint32_t)jpeg0_irq_handler);
        NVIC_SetPriority(JPEG_DEC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(JPEG_DEC_IRQn);
        NVIC_EnableIRQ(JPEG_DEC_IRQn);
    }
#ifdef JPEG1_BASE
    else if (id == HAL_JPEG_ID_1) {
        NVIC_SetVector(JPEG1_DEC_IRQn, (uint32_t)jpeg1_irq_handler);
        NVIC_SetPriority(JPEG1_DEC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(JPEG1_DEC_IRQn);
        NVIC_EnableIRQ(JPEG1_DEC_IRQn);
    }
#endif
}

static void hal_jpeg_irq_disable(enum HAL_JPEG_ID_T id)
{
    if (id == HAL_JPEG_ID_0) {
        NVIC_ClearPendingIRQ(JPEG_DEC_IRQn);
        NVIC_DisableIRQ(JPEG_DEC_IRQn);
    }
#ifdef JPEG1_BASE
    else if (id == HAL_JPEG_ID_1) {
        NVIC_ClearPendingIRQ(JPEG1_DEC_IRQn);
        NVIC_DisableIRQ(JPEG1_DEC_IRQn);
    }
#endif
}

static int hal_jpeg_start_decode(enum HAL_JPEG_ID_T id, struct HAL_JPEG_CONFIG_T *cfg)
{
    struct JPEG_REG_T *jpeg;
    struct JPEG_CODEC_T *codec;
    uint8_t val = 0;
    int dec_fmt = 0;
    int mcu_size = 0;
    int mcu_cnt = 0;
    int ydu_cnt = 0;
    int udu_cnt = 0;
    int vdu_cnt = 0;

    if (id >= HAL_JPEG_ID_QTY)
        return HAL_JPEG_ERR;
    if (!cfg)
        return HAL_JPEG_ERR;
    if (!cfg->pix || !cfg->ecs || !cfg->qtable ||
        !cfg->hbase || !cfg->hmin || !cfg->hsymb)
        return HAL_JPEG_ERR;

#ifdef CHIP_BEST1503
    if ((hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6) &&
        (cfg->format == HAL_JPEG_YUV420 || cfg->format == HAL_JPEG_YUV444))
        return HAL_JPEG_NOT_SUPPORT;
#endif

    jpeg = jpeg_base[id];
    codec = &jpeg_codec[id];

    if (!codec->opened)
        return HAL_JPEG_NOT_OPENED;
    if (codec->working)
        return HAL_JPEG_ERR;

    switch (cfg->format) {
        case HAL_JPEG_YUV444:
            dec_fmt = 2;
            mcu_size = 8*8;
            ydu_cnt = 1;
            udu_cnt = 1;
            vdu_cnt = 1;
            break;
        case HAL_JPEG_YUV420:
            dec_fmt = 1;
            mcu_size = 16*16;
            ydu_cnt = 4;
            udu_cnt = 1;
            vdu_cnt = 1;
            break;
        case HAL_JPEG_YUV422:
        default:
            dec_fmt = 0;
            mcu_size = 16*8;
            ydu_cnt = 2;
            udu_cnt = 1;
            vdu_cnt = 1;
            break;
    }

    /* Reset codec to clear working status */
    if (id == HAL_JPEG_ID_0) {
        hal_cmu_reset_set(HAL_CMU_MOD_H_JPEG);
        hal_sys_timer_delay(1);
        hal_cmu_reset_clear(HAL_CMU_MOD_H_JPEG);
    }
#ifdef JPEG_BASE1
    else if (id == HAL_JPEG_ID_1) {
        hal_cmu_reset_set(HAL_CMU_MOD_H_JPEG1);
        hal_sys_timer_delay(1);
        hal_cmu_reset_clear(HAL_CMU_MOD_H_JPEG1);
    }
#endif

    /* The output pixel data only support YUV422 packed.
     * YUV444/422/420 planar data will convert to YUV422 packed by Codec.
     */
    cfg->pix_size = cfg->width * cfg->height * 2;

    mcu_cnt = (cfg->width * cfg->height) / mcu_size;

    jpeg->REG_04 = JPEG_DEC_MODE | JPEG_DEC_FORMAT(dec_fmt);

    if (cfg->swap_yu)
        jpeg->REG_04 |= JPEG_DEC_SWAP_YU;
    if (cfg->swap_uv)
        jpeg->REG_04 |= JPEG_DEC_SWAP_UV;

    /* The decoded YUV444/422/420 planar pixel data size */
    if (cfg->format == HAL_JPEG_YUV444)
        codec->dec_size = cfg->width * cfg->height * 3;
    else if (cfg->format == HAL_JPEG_YUV422)
        codec->dec_size = cfg->width * cfg->height * 2;
    else
        codec->dec_size = cfg->width * cfg->height * 3 / 2;

    /* If the total decoded pixel size is larger than JPEG_DEC_BLK_SZ,
     * transfer pixel data multiple times.
     */
    if (codec->dec_size > JPEG_DEC_BLK_SZ) {
        jpeg->REG_10 = JPEG_PIX_XFER_LEN(JPEG_DEC_BLK_SZ);
        codec->dec_size -= JPEG_DEC_BLK_SZ;
    } else {
        jpeg->REG_10 = JPEG_PIX_XFER_LEN(codec->dec_size);
        codec->dec_size = 0;
    }

    jpeg->REG_14 = JPEG_ECS_XFER_LEN(cfg->ecs_size);
    jpeg->REG_18 = JPEG_TOTAL_INCOM_LEN(cfg->ecs_size);

    jpeg->REG_20 = JPEG_DUMMY_MODE(0);

    jpeg->REG_60 = JPEG_FIX_BUG_EN;
    jpeg->REG_70 = JPEG_DEC_PIC_WIDTH(cfg->width-1) | JPEG_DEC_PIC_HEIGHT(cfg->height-1);

    val = 0xf;
#ifdef CHIP_BEST1503
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        val = 0x7;
#endif
    jpeg->REG_84 = JPEG_ECS_REQ_THRESHOLD(val) | JPEG_PIX_REQ_THRESHOLD(val);

    jpeg->REG_1004 = JPEG_CODEC_NCOL(2) | JPEG_CODEC_DEC_MODE; /* color num - 1, for YUV: 3 - 1 */
    jpeg->REG_1008 = JPEG_CODEC_NMCU(mcu_cnt-1);
    if (cfg->rst_nmcu > 0) {
        jpeg->REG_1004 |= JPEG_CODEC_RESTART_MARKER_EN;
        jpeg->REG_100C = JPEG_CODEC_NRST(cfg->rst_nmcu - 1);
    }
    jpeg->REG_1010 = JPEG_CODEC_NBLOCK0(ydu_cnt-1) | /* Y-DU in MCU, num-1 */
                     JPEG_CODEC_HD0(cfg->hdt[0]) |
                     JPEG_CODEC_HA0(cfg->hat[0]) |
                     JPEG_CODEC_QT0(cfg->qt[0]);
    jpeg->REG_1014 = JPEG_CODEC_NBLOCK1(udu_cnt-1) | /* U-DU in MCU, num-1 */
                     JPEG_CODEC_HD1(cfg->hdt[1]) |
                     JPEG_CODEC_HA1(cfg->hat[1]) |
                     JPEG_CODEC_QT1(cfg->qt[1]);
    jpeg->REG_1018 = JPEG_CODEC_NBLOCK2(vdu_cnt-1) | /* V-DU in MCU, num-1 */
                     JPEG_CODEC_HD2(cfg->hdt[2]) |
                     JPEG_CODEC_HA2(cfg->hat[2]) |
                     JPEG_CODEC_QT2(cfg->qt[2]);

    hal_jpeg_write_qtable(id, cfg->qtable, cfg->qtable_size);
    hal_jpeg_write_hbase(id, cfg->hbase, cfg->hbase_size);
    hal_jpeg_write_hmin(id, cfg->hmin, cfg->hmin_size);
    hal_jpeg_write_hsymb(id, cfg->hsymb, cfg->hsymb_size);

    codec->cfg = cfg;
    codec->working = true; /// 20260312. root，修复小图解码概率失败问题

    hal_jpeg_irq_enable(id, cfg->mode);

    jpeg->REG_1000 = JPEG_CODEC_START;
    jpeg->REG_08 = JPEG_PIX_XFER_START;
    jpeg->REG_0C = JPEG_ECS_XFER_START;

    if (hal_jpeg_pix_rx(id, cfg->pix, cfg->pix_size))
        return HAL_JPEG_PIX_DMA_ERR;

    if (hal_jpeg_ecs_tx(id, cfg->ecs, cfg->ecs_size))
        return HAL_JPEG_ECS_DMA_ERR;

    return HAL_JPEG_OK;
}

static int hal_jpeg_start_encode(enum HAL_JPEG_ID_T id, struct HAL_JPEG_CONFIG_T *cfg)
{
    struct JPEG_REG_T *jpeg;
    struct JPEG_CODEC_T *codec;
    int mcu_size = 0;
    int mcu_cnt = 0;
    int ydu_cnt = 0;
    int udu_cnt = 0;
    int vdu_cnt = 0;

#if defined(CHIP_BEST1503) || defined(CHIP_BEST1700) || defined(CHIP_BEST1306P)
    return HAL_JPEG_NOT_SUPPORT;
#endif

    if (id >= HAL_JPEG_ID_QTY)
        return HAL_JPEG_ERR;

    if (!cfg)
        return HAL_JPEG_ERR;
    if (!cfg->pix || !cfg->ecs || !cfg->qtable || !cfg->hetable)
        return HAL_JPEG_ERR;

    jpeg = jpeg_base[id];
    codec = &jpeg_codec[id];

    if (!codec->opened)
        return HAL_JPEG_NOT_OPENED;
    if (codec->working)
        return HAL_JPEG_ERR;

    /* Encode only support YUV422 */
    if (cfg->format != HAL_JPEG_YUV422)
        return HAL_JPEG_NOT_SUPPORT;
    mcu_size = 16*8;
    ydu_cnt = 2;
    udu_cnt = 1;
    vdu_cnt = 1;

    /* Reset codec to clear working status */
    if (id == HAL_JPEG_ID_0) {
        hal_cmu_reset_set(HAL_CMU_MOD_H_JPEG);
        hal_sys_timer_delay(1);
        hal_cmu_reset_clear(HAL_CMU_MOD_H_JPEG);
    }
#ifdef JPEG_BASE1
    else if (id == HAL_JPEG_ID_1) {
        hal_cmu_reset_set(HAL_CMU_MOD_H_JPEG1);
        hal_sys_timer_delay(1);
        hal_cmu_reset_clear(HAL_CMU_MOD_H_JPEG1);
    }
#endif

    /* Assume compression ratio is 1:4 */
    cfg->ecs_size = cfg->pix_size/4;

    mcu_cnt = (cfg->width * cfg->height) / mcu_size;

    jpeg->REG_04 = JPEG_ENC_MODE;

    if (cfg->swap_yu)
        jpeg->REG_04 |= JPEG_ENC_SWAP_YU;
    if (cfg->swap_uv)
        jpeg->REG_04 |= JPEG_ENC_SWAP_UV;

    jpeg->REG_10 = JPEG_PIX_XFER_LEN(cfg->pix_size);
    //jpeg->REG_14 = JPEG_ECS_XFER_LEN(cfg->ecs_size);
    jpeg->REG_18 = JPEG_TOTAL_INCOM_LEN(cfg->pix_size);

    jpeg->REG_20 = JPEG_DUMMY_MODE(0);

    jpeg->REG_60 = 0;
    jpeg->REG_74 = JPEG_ENC_PIC_WIDTH(cfg->width) | JPEG_ENC_PIC_HEIGHT(cfg->height) |
                   JPEG_ENC_XMODE_EN;
    jpeg->REG_84 = JPEG_ECS_REQ_THRESHOLD(0x7) | JPEG_PIX_REQ_THRESHOLD(0x7);

    jpeg->REG_1004 = JPEG_CODEC_NCOL(2) | JPEG_CODEC_ENC_MODE; /* color num - 1, for YUV: 3 - 1 */
    jpeg->REG_1008 = JPEG_CODEC_NMCU(mcu_cnt-1);
    if (cfg->rst_nmcu > 0) {
        jpeg->REG_1004 |= JPEG_CODEC_RESTART_MARKER_EN;
        jpeg->REG_100C = JPEG_CODEC_NRST(cfg->rst_nmcu - 1);
    }
    jpeg->REG_1010 = JPEG_CODEC_NBLOCK0(ydu_cnt-1) | /* Y-DU in MCU, num-1 */
                     JPEG_CODEC_HD0(cfg->hdt[0]) |
                     JPEG_CODEC_HA0(cfg->hat[0]) |
                     JPEG_CODEC_QT0(cfg->qt[0]);
    jpeg->REG_1014 = JPEG_CODEC_NBLOCK1(udu_cnt-1) | /* U-DU in MCU, num-1 */
                     JPEG_CODEC_HD1(cfg->hdt[1]) |
                     JPEG_CODEC_HA1(cfg->hat[1]) |
                     JPEG_CODEC_QT1(cfg->qt[1]);
    jpeg->REG_1018 = JPEG_CODEC_NBLOCK2(vdu_cnt-1) | /* V-DU in MCU, num-1 */
                     JPEG_CODEC_HD2(cfg->hdt[2]) |
                     JPEG_CODEC_HA2(cfg->hat[2]) |
                     JPEG_CODEC_QT2(cfg->qt[2]);

    hal_jpeg_write_qtable(id, cfg->qtable, cfg->qtable_size);
    hal_jpeg_write_hetable(id, cfg->hetable, cfg->hetable_size);

    hal_jpeg_irq_enable(id, cfg->mode);

    jpeg->REG_1000 = JPEG_CODEC_START;
    jpeg->REG_08 = JPEG_PIX_XFER_START;
    jpeg->REG_0C = JPEG_ECS_XFER_START;

    if (hal_jpeg_ecs_rx(id, cfg->ecs, cfg->ecs_size))
        return HAL_JPEG_ECS_DMA_ERR;

    if (hal_jpeg_pix_tx(id, cfg->pix, cfg->pix_size))
        return HAL_JPEG_PIX_DMA_ERR;

    codec->cfg = cfg;
    codec->working = true;
    return HAL_JPEG_OK;
}

int hal_jpeg_start(enum HAL_JPEG_ID_T id, struct HAL_JPEG_CONFIG_T *cfg)
{
    if (cfg->mode == HAL_JPEG_ENC_MODE)
        return hal_jpeg_start_encode(id, cfg);
    else
        return hal_jpeg_start_decode(id, cfg);
}

int hal_jpeg_stop(enum HAL_JPEG_ID_T id)
{
    struct JPEG_REG_T *jpeg;
    struct JPEG_CODEC_T *codec;

    if (id >= HAL_JPEG_ID_QTY)
        return HAL_JPEG_ERR;

    jpeg = jpeg_base[id];
    codec = &jpeg_codec[id];

    codec->working = false;
    codec->cfg = NULL;

    hal_jpeg_irq_disable(id);

    jpeg->REG_0C &= ~JPEG_ECS_XFER_START;
    jpeg->REG_08 &= ~JPEG_PIX_XFER_START;
    jpeg->REG_1000 &= ~JPEG_CODEC_START;

    if (codec->pix_ch >= 0) {
        hal_dma_free_chan(codec->pix_ch);
        codec->pix_ch = -1;
    }
    if (codec->ecs_ch >= 0) {
        hal_dma_free_chan(codec->ecs_ch);
        codec->ecs_ch = -1;
    }

    return HAL_JPEG_OK;
}

bool hal_jpeg_busy(enum HAL_JPEG_ID_T id)
{
    if (id >= HAL_JPEG_ID_QTY)
        return false;
    return jpeg_codec[id].working;
}

#endif /* JPEG_BASE */
