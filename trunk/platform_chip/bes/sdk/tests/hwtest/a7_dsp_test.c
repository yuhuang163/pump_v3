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
#ifdef CHIP_HAS_A7_DSP
#ifdef A7_DSP_TEST

#include "hwtest.h"
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "string.h"

extern uint32_t __dsp_code_start[];
extern uint32_t __dsp_code_end[];

void a7_dsp_test(void)
{
    uint32_t entry;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_RET_T dma_ret;
    uint32_t remains;
    uint32_t size;

    TRACE(1, "%s", __FUNCTION__);

    hal_cmu_dsp_clock_enable();
    hal_cmu_dsp_reset_clear();

    entry = *(__dsp_code_end - 1);
    remains = __dsp_code_end - __dsp_code_start - 1;
    size = remains;

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_periph = HAL_GPDMA_MEM;
    dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = NULL;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_GPDMA_MEM;
    dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.type = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg.try_burst = false;
    dma_cfg.ch = hal_dma_get_chan(dma_cfg.dst_periph, HAL_DMA_LOW_PRIO);
    ASSERT(dma_cfg.ch != HAL_DMA_CHAN_NONE, "%s: Failed to get dma chan", __func__);

    dma_cfg.dst = entry;
    dma_cfg.src = (uint32_t)__dsp_code_start;

    while (remains > 0) {
        if (remains > HAL_DMA_MAX_DESC_XFER_SIZE) {
            dma_cfg.src_tsize = HAL_DMA_MAX_DESC_XFER_SIZE;
        } else {
            dma_cfg.src_tsize = remains;
        }
        dma_ret = hal_dma_start(&dma_cfg);
        ASSERT(dma_ret == HAL_DMA_OK, "%s: Failed to start dma: %d", __func__, dma_ret);

        while (hal_dma_chan_busy(dma_cfg.ch));

        dma_cfg.dst += dma_cfg.src_tsize * 4;
        dma_cfg.src += dma_cfg.src_tsize * 4;
        remains -= dma_cfg.src_tsize;
    }

    hal_dma_sync_for_device((uint32_t *)entry, size);

    hal_dma_free_chan(dma_cfg.ch);

    hal_cmu_dsp_init_boot_reg(entry | 1);

    // Stop using UART and let DSP take the control
    //hal_trace_flush_buffer();
    //hal_trace_close();

    // Stop handling any IRQs
    //int_lock();

    hal_cmu_dsp_start_cpu();

    //switch to a7 jtag
    //*(volatile uint32_t*)0x40000140 |= (0xff<<11);
    //*(volatile uint32_t*)0x4000005c |= (1<<26);
}

#endif
#endif

