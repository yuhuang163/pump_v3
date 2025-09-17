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
#ifdef TEST_DMA
#include "hal_trace.h"
#include "cmsis.h"
#include "hal_dma.h"
#include <string.h>

#define _BUF_MAX                            (1024)
#define DMA_TYPE                            (HAL_DMA_FLOW_M2M_DMA)

/* DMA source config. */
#ifndef SRC_BUF_MAX
#define SRC_BUF_MAX                         (_BUF_MAX)
#endif
#ifndef DMA_SRC_BUSRT_SIZE
#define DMA_SRC_BUSRT_SIZE                  (HAL_DMA_BSIZE_4)
#endif
#ifndef DMA_SRC_XFER_WIDTH
#define DMA_SRC_XFER_WIDTH                  (HAL_DMA_WIDTH_WORD)
#endif
#define DMA_SRC_PERIPH                      (HAL_DMA_PERIPH_NULL)

/* DMA destination config. */
#define DST_BUF_MAX                         (_BUF_MAX)
#define DMA_DST_BUSRT_SIZE                  (HAL_DMA_BSIZE_4)
#define DMA_DST_XFER_WIDTH                  (HAL_DMA_WIDTH_WORD)
#define DMA_DST_PERIPH                      (HAL_DMA_PERIPH_NULL)

#define DMA_PERIPH                          (HAL_GPDMA_MEM)

/* Terminal count interrupt enable. enable if 1, and disable or not. */
#define DMA_TC_IRQ_ENABLE                   (1)

static struct HAL_DMA_DESC_T g_dma_desc = {0};
static uint32_t src_buf[SRC_BUF_MAX] = {0};
static uint32_t dst_buf[DST_BUF_MAX] = {0};

static void dma_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    if (error) {             /* error counter Interrupt */
        // TODO:
        TRACE(3,"%s error counter Interrupt chan[%d] remain_tsize[%d]", __FUNCTION__, chan, remain_tsize);
    } else {                /* terminate counter Interrupt */
        // TODO:
        /* release the DMA channel. */
        hal_dma_free_chan(chan);
        TRACE(3,"%s terminate counter Interrupt chan[%d] remain_tsize[%d]", __FUNCTION__, chan, remain_tsize);

        uint32_t i = 0;
        for (i=0; i<DST_BUF_MAX; i+=8) {
                TRACE(3,"%08x %08x %08x %08x %08x %08x %08x %08x", 
                            dst_buf[i+0], dst_buf[i+1], dst_buf[i+2], dst_buf[i+3],
                            dst_buf[i+4], dst_buf[i+5], dst_buf[i+6], dst_buf[i+7]
                    );
        }
    }
}

static void dma_start_cb(uint8_t chan)
{
    TRACE(3,"%s chan[%d] used.", __FUNCTION__, chan);
}

void test_dma(void)
{
    enum HAL_DMA_RET_T ret = HAL_DMA_OK;

    /* dst buffers are cleared. */
    /* src buffers are setted. */
    uint32_t i = 0;
    for (i = 0; i < SRC_BUF_MAX; i++) {
        dst_buf[i] = 0;
        src_buf[i] = (i<<16) | i;
    }

    /* */
    hal_dma_open();

    /* desc config */
    struct HAL_DMA_CH_CFG_T cfg = {0};
    //memset((void *)&cfg, 0, ARRAY_SIZE(cfg));

    /* type */
    cfg.type = DMA_TYPE;

    /* src config. */
    cfg.src = (uint32_t)src_buf;                            /* src addr */
    cfg.src_tsize = SRC_BUF_MAX / DMA_SRC_BUSRT_SIZE;  
    cfg.src_bsize = DMA_SRC_BUSRT_SIZE;                     /* src burst size */
    cfg.src_width = DMA_SRC_XFER_WIDTH;                     /* src transfer width */
    cfg.src_periph = DMA_SRC_PERIPH;                        /* ??? */

    /* dst config. */
    cfg.dst = (uint32_t)dst_buf;                            /* destination addr */
    cfg.dst_bsize = DMA_DST_BUSRT_SIZE;                     /* dst burst size */
    cfg.dst_width = DMA_DST_XFER_WIDTH;                     /* dst xfer width */
    cfg.dst_periph = DMA_DST_PERIPH;

    cfg.try_burst = 1;                                      /* burst mode??? */
    cfg.handler = dma_handler;
    cfg.start_cb = dma_start_cb;
    cfg.ch = hal_dma_get_chan(DMA_PERIPH, HAL_DMA_HIGH_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        TRACE(3,"%s all channel can be busy.", __FUNCTION__);
        return;
    }

    /* cofig: g_dma_desc. src, dst, lli, and ctrl */
    ret = hal_dma_init_desc(&g_dma_desc, &cfg, NULL, DMA_TC_IRQ_ENABLE);
    if (ret) {
        TRACE(3,"%s hal_dma_init_desc fail.", __FUNCTION__);
    }

    /* start */
    ret = hal_dma_sg_start(&g_dma_desc, &cfg);
    if (ret) {
        TRACE(3,"%s hal_dma_sg_start fail.", __FUNCTION__);
    }
}

#endif
