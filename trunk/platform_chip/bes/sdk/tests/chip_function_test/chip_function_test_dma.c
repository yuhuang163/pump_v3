/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef CHIP_FUNCTION_TEST_DMA
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_trace.h"
 #include "string.h"
#ifdef SENSOR_TEST
#include "sensor_test.h"
#endif

#ifdef RTOS
#include "cmsis_os.h"
#ifdef KERNEL_RTX
#include "rt_Time.h"
#endif
#endif
#include "hal_dma.h"
#define DMA_TEST_BUFFER_LEN (8*1024)
#ifdef CHIP_FUNCTION_TEST_NORFLASH
extern uint8_t buff_r[];
extern uint8_t buff_b[];
uint32_t *src_buff;
uint32_t *dst_buff;
#else
uint32_t src_buff[DMA_TEST_BUFFER_LEN];
uint32_t dst_buff[DMA_TEST_BUFFER_LEN];
#endif
bool mda_test_flag;
uint32_t mda_test_remain_dst_size =0;

// DMA IRQ Handle.
static void DMA_IRQHandler(uint8_t chan, uint32_t remain_dst_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static uint32_t  index = 0;
    TRACE(1, "%s: I am dma irq, I'm coming!!! index = %d", __func__, index++);
}

static void _dma_test(uint32_t src, uint32_t dst, uint32_t len)
{
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_RET_T dma_ret;
    uint32_t remain;

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_periph = HAL_GPDMA_MEM;
    dma_cfg.ch = hal_dma_get_chan(dma_cfg.dst_periph, HAL_DMA_LOW_PRIO);
    ASSERT (dma_cfg.ch != HAL_DMA_CHAN_NONE, "Failed to alloc dma ch");
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = DMA_IRQHandler;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_GPDMA_MEM;
    dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.type = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg.try_burst = false;

    dma_cfg.dst = dst;
    dma_cfg.src = src;

    remain = len;
    while (remain > 0) {
        if (remain > HAL_DMA_MAX_DESC_XFER_SIZE) {
            dma_cfg.src_tsize = HAL_DMA_MAX_DESC_XFER_SIZE;
        } else {
            dma_cfg.src_tsize = remain;
        }
        mda_test_flag = 0;
        dma_ret = hal_dma_start(&dma_cfg);
        ASSERT(dma_ret == HAL_DMA_OK, "%s: Failed to start dma: %d", __func__, dma_ret);

        while (hal_dma_chan_busy(dma_cfg.ch));
        dma_cfg.dst += dma_cfg.src_tsize * 4;
        dma_cfg.src += dma_cfg.src_tsize * 4;
        remain -= dma_cfg.src_tsize;
        osDelay(10);
    }

    hal_dma_free_chan(dma_cfg.ch);
}

void function_test_dma(void)
{
    uint32_t len = DMA_TEST_BUFFER_LEN;
    uint32_t i;

    TRACE(1, "DMA TEST:0x%x  START", len);
    osDelay(4);
#ifdef CHIP_FUNCTION_TEST_NORFLASH
    src_buff = (uint32_t*)buff_r;
    dst_buff = (uint32_t*)buff_b;
#endif
    for(i = 0; i < len; i++)
    {
        src_buff[i] = i;
    }
    while(len > 0)
    {
        memset(&dst_buff[0],0, DMA_TEST_BUFFER_LEN*sizeof(uint32_t));
       // osTimerStart(dma_test_timer_id,DMA_TEST_DELAY_MS);
        _dma_test((uint32_t)src_buff, (uint32_t)dst_buff, len);
        for(i = 0; i < len; i++)
        {
            if(dst_buff[i] != i)
            {
                TRACE(1, "dst_buff[%d] = 0x%08x error", i, dst_buff[i]);
                goto _func_error;
            }
        }
        TRACE(1, "DMA :len = 0x%08x DONE", len);
        osDelay(4);
        len = (len>>1);
    }

    TRACE(1, "DMA TEST: DONE.");
    osDelay(10);
    while(1);
_func_error:

    TRACE(1, "DMA TEST: NG!");
    osDelay(10);
    while(1);
}
#endif
