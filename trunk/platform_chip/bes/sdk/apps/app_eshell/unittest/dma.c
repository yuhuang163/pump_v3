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

#if defined(UTILS_ESHELL_EN)
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "eshell.h"
#include "hal_dma.h"
#include "app_eshell.h"

uint32_t try_cnt = 0;
static uint32_t cnt = 0;
static uint32_t tranf_bytes = 0;
static uint32_t src_addr;
static uint32_t dst_addr;
static struct HAL_DMA_DESC_T dma_desc = {0};

static void test_dma_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    cnt++;
    if (error == 0) {
        eshell_putstring("test_dma_handler, DMA transfer completed cnt=%d\r\n", cnt);
    } else {
        eshell_putstring("test_dma_handler, DMA transfer remain_tsize=%d err=%d cnt=%d\r\n", remain_tsize, error, cnt);
    }

    if (try_cnt == cnt) {
        cnt = 0;
    }

    hal_dma_free_chan(chan);
}

static void test_dma(uint32_t src_addr, uint32_t dst_addr, uint16_t src_tsize, enum HAL_DMA_BSIZE_T burst_size, enum HAL_DMA_WIDTH_T width)
{
    uint32_t ret;
    uint8_t *buf;
    struct HAL_DMA_CH_CFG_T cfg = {0};

    memset((uint8_t *)dst_addr, 0, tranf_bytes);
    buf = (uint8_t *)src_addr;
    for (int i = 0; i < tranf_bytes; i++) {
        buf[i] = (uint8_t)i;
    }

    hal_dma_open();
    /* type */
    cfg.type = HAL_DMA_FLOW_M2M_DMA;

    /* src config. */
    cfg.src_bsize = burst_size;

    /* dst config. */
    cfg.dst = dst_addr;
    cfg.dst_periph = 0;
    cfg.dst_bsize = burst_size;
    cfg.dst_width = width;

    cfg.try_burst = 1;
    cfg.handler = test_dma_handler;
    cfg.start_cb = NULL;
    cfg.ch = hal_dma_get_chan(HAL_GPDMA_MEM, HAL_DMA_HIGH_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        eshell_putstring("all DMA channel busy\r\n");
        return;
    }

    ret = hal_dma_init_desc(&dma_desc, &cfg, NULL, 1);
    if (ret) {
        eshell_putstring("hal_dma_init_desc fail, ret=%d\r\n", ret);
    }

    ret = hal_dma_sg_start(&dma_desc, &cfg);
    if (ret) {
        eshell_putstring("hal_dma_sg_start fail, ret=%d\r\n", ret);
    }
}

static void utest_dma(int argc, char *argv[])
{
    if (argc != 7) {
        eshell_putstring("Usage\r\n");
        eshell_putstring("utest_dma src_addr(hex) dst_addr(hex) size burst_size width try_cnt\r\n");
        return;
    }

    uint32_t src_tsize = atoi(argv[3]);
    enum HAL_DMA_BSIZE_T burst_size = atoi(argv[4]);
    enum HAL_DMA_WIDTH_T width = atoi(argv[5]);
    try_cnt = atoi(argv[6]);
    sscanf(argv[1], "%x", &src_addr);
    sscanf(argv[2], "%x", &dst_addr);
    src_addr = ((src_addr % 4) == 0) ? src_addr : (src_addr + 4 - (src_addr % 4));
    dst_addr = ((dst_addr % 4) == 0) ? dst_addr : (dst_addr + 4 - (dst_addr % 4));

    if (src_tsize < 1)
    {
        eshell_putstring("src_tsize err, minimum value: 1\r\n");
        return;
    }

    if ((burst_size < 0) || (burst_size > 7)) {
        eshell_putstring("burst_size err, valid value: 0 ~ 7\r\n");
        return;
    }

    if ((width < 0) || (width > 2)) {
        eshell_putstring("width err, valid value: 0 ~ 2, 0: byte 1:half word 2:word\r\n");
        return;
    }

    if (width == 0) {
        tranf_bytes = src_tsize;
    } else if (width == 1) {
        tranf_bytes = src_tsize * 2;
    } else if (width == 2) {
        tranf_bytes = src_tsize * 4;
    }

    eshell_putstring("src_addr=%#x, dst_addr=%#x, src_tsize=%d, burst_size=%d, width=%d\r\n",src_addr, dst_addr, src_tsize, burst_size, width);
    for (uint8_t i = 0; i < try_cnt; i++) {
        test_dma(src_addr, dst_addr, src_tsize, burst_size, width);
    }

}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_dma", "usage: utest_dma help",
                   utest_dma);
#endif
