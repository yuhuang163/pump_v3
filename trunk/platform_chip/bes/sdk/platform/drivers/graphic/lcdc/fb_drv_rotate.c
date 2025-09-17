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
 *
 ****************************************************************************/

#include <stdio.h>
#include "string.h"
#include "cmsis.h"
#include "lcdc.h"
#include "hal_lcdc.h"
#include "hal_dsi.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "hal_dma2d.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif

#include "hal_trace.h"
#include "gra_sem.h"
#include "lpcmd.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_location.h"
#if defined(__NuttX__)
#include <nuttx/config.h>
#include <nuttx/video/fb.h>
#include <arch/board/board.h>
#endif
//#define GRAPHIC_DRV_DEBUG
//#define GRAPHIC_DRV_VERBOSE

#ifdef  GRAPHIC_DRV_VERBOSE
#ifndef GRAPHIC_DRV_DEBUG
#define GRAPHIC_DRV_DEBUG
#endif
#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)
#define LCDC_REGINFO(...)  TR_INFO( TR_MOD(TEST), __VA_ARGS__)
#else
#define LCDC_INFO(...)
#define LCDC_REGINFO(...)
#endif

#ifdef  GRAPHIC_DRV_DEBUG
#define LCDC_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define LCDC_REP(...)
#endif

#define LCDC_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#ifdef CONFIG_FB_ROTATE_270
#define ROTATE_270
#endif

#define DMA_COUNT   2
SYNC_FLAGS_LOC static struct HAL_DMA_CH_CFG_T dma_cfg[DMA_COUNT];
SYNC_FLAGS_LOC static struct HAL_DMA_DESC_T desc_bank[DMA_COUNT][128];
static POSSIBLY_UNUSED sem_t covert_done[DMA_COUNT];
static uint32_t dma_ch[DMA_COUNT];
static uint32_t dma_ch_mask;
static uint32_t start_tick;
static DMA_DONE_IRQ_CALLBACK dma_done_irq_cb;
static uint32_t dma_dst_buff;
static uint32_t dma_src_buff;

#if 0
static void hw_wait_complete()
{
    for (int k=0; k < DMA_COUNT; k++)
    {
        gra_sem_wait(&covert_done[k]);
        hal_dma_free_chan(dma_ch[k]);
    }
}
#endif

static void dma_irq_handler_covert(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    LCDC_INFO("%s ch:%d,dma_ch_mask:0x%x", __func__,chan,dma_ch_mask);
    uint32_t lk = int_lock();
    hal_dma_free_chan(chan);
    dma_ch_mask &= ~(1<<chan);
    if (dma_ch_mask == 0)
    {
        if (dma_done_irq_cb != NULL)
            dma_done_irq_cb((void *)dma_dst_buff);
        LCDC_REP("Commit buffer = %p -> %p  %" PRIu32 " us\n",
                  (void *)dma_src_buff,(void *)dma_dst_buff,
                  FAST_TICKS_TO_US(hal_fast_sys_timer_get()-start_tick));
    }
    int_unlock(lk);
}

void dma2d_rotate_90(uint32_t src_ptr, uint32_t dst_ptr,uint32_t src_width, uint32_t src_hight, DMA_DONE_IRQ_CALLBACK irq_cb)
{
    enum HAL_DMA_RET_T ret;

    dma_dst_buff = dst_ptr;
    dma_src_buff = src_ptr;
    dma_done_irq_cb = irq_cb;

#if defined(CONFIG_ROTATE_DST2D)
    uint32_t dma_line = HAL_DMA_MAX_DESC_XFER_SIZE/src_width;
    uint32_t dma_frame_sz = dma_line*src_width;

    uint32_t len = src_width*src_hight/DMA_COUNT;
    uint32_t desc_cnt = (len + dma_frame_sz - 1) / dma_frame_sz;
    if (desc_cnt > 128) {
        ASSERT(false, "%s: alloc memeory(size:%d) fail", __func__,
               sizeof(struct HAL_DMA_DESC_T) * desc_cnt);
    }
    if (dma_frame_sz > src_width*src_hight/DMA_COUNT)
        dma_frame_sz = src_width*src_hight/DMA_COUNT;

    LCDC_REP("%s: src_width :%d src_hight :%d", __func__,src_width,src_hight);
    LCDC_INFO("%s: desc_cnt=%d,src_ptr=0x%x,dst_ptr=0x%x", __func__,desc_cnt,src_ptr,dst_ptr);

#ifdef ROTATE_270
    struct HAL_DMA_2D_CFG_T dest_2d;
    dest_2d.xmodify = - src_hight-1;
    dest_2d.xcount = src_width;
    dest_2d.ymodify = (src_width)*(src_hight) - src_hight;
    dest_2d.ycount = src_hight/DMA_COUNT;
#else
    struct HAL_DMA_2D_CFG_T dest_2d;
    dest_2d.xmodify = src_hight-1;
    dest_2d.xcount = src_width;
    dest_2d.ymodify = 0 - (src_width)*(src_hight) + src_hight -2;
    dest_2d.ycount = src_hight/DMA_COUNT;
#endif

    start_tick = hal_fast_sys_timer_get();
    for (int k=0; k < DMA_COUNT; k++)
    {
        dma_ch[k] = hal_gpdma_get_chan(HAL_GPDMA_MEM, HAL_DMA_HIGH_PRIO);
        if (dma_ch[k] == HAL_DMA_CHAN_NONE) {
            LCDC_ERR("%s %d, ch[%d] eror", __func__, __LINE__,k);
            return;
        }
        dma_ch_mask |= (1<<dma_ch[k]);

        dma_cfg[k].handler = dma_irq_handler_covert;
        dma_cfg[k].dst_bsize = HAL_DMA_BSIZE_1;
        dma_cfg[k].dst_periph = HAL_GPDMA_MEM;
        dma_cfg[k].dst_width = FB_PIX_BYTE/2;
        dma_cfg[k].src_bsize = HAL_DMA_BSIZE_256;
        dma_cfg[k].src_periph = HAL_GPDMA_MEM;
        dma_cfg[k].src_width = FB_PIX_BYTE/2;
        dma_cfg[k].type = HAL_DMA_FLOW_M2M_DMA;
        dma_cfg[k].try_burst = 1;
        dma_cfg[k].ch = dma_ch[k];
        gra_sem_init(&covert_done[k], 0, 0);
        LCDC_INFO("%s: dma_ch=%d", __func__,dma_ch[k]);

        struct HAL_DMA_DESC_T *desc = &desc_bank[k][0];

        dma_cfg[k].src = src_ptr+k*src_width*FB_PIX_BYTE*src_hight/DMA_COUNT;
        dma_cfg[k].src_tsize = dma_frame_sz;
        int i = 0;
        for (i = 0; i < desc_cnt - 1; i++) {
#ifdef ROTATE_270
            dma_cfg[k].dst =  dst_ptr +  (src_width*src_hight - src_hight + k*src_hight/DMA_COUNT + i*dma_line)*FB_PIX_BYTE;
#else
            dma_cfg[k].dst =  dst_ptr  + (src_hight- dma_line*i -1)*FB_PIX_BYTE - k*FB_PIX_BYTE*src_hight/DMA_COUNT;
#endif
            LCDC_INFO("%s: dmalink :%d dst=0x%x", __func__,i,dma_cfg[k].dst);
            ret =
                hal_dma_init_desc(&desc[i], &dma_cfg[k], &desc[i + 1], 0);
            if (ret != HAL_DMA_OK) {
                LCDC_ERR("%s:1 ret=%d", __func__,ret);
                return;
            }
            dma_cfg[k].src += dma_frame_sz*FB_PIX_BYTE;
        }
#ifdef ROTATE_270
        dma_cfg[k].dst =  dst_ptr +  (src_width*src_hight - src_hight + k*src_hight/DMA_COUNT + i*dma_line)*FB_PIX_BYTE;
#else
        dma_cfg[k].dst =  dst_ptr  + (src_hight- dma_line*i -1)*FB_PIX_BYTE - k*FB_PIX_BYTE*src_hight/DMA_COUNT;
#endif
        dma_cfg[k].src_tsize = (src_ptr+(k+1)*src_width*FB_PIX_BYTE*src_hight/DMA_COUNT - dma_cfg[k].src)/FB_PIX_BYTE;
        LCDC_INFO("%s: dmalink :%d dst=0x%x,src_tsize=%d", __func__,i,dma_cfg[k].dst,dma_cfg[k].src_tsize);
        ret = hal_dma_init_desc(&desc[i], &dma_cfg[k], NULL, 1);
        if (ret != HAL_DMA_OK) {
            LCDC_ERR("%s:2 ret=%d", __func__,ret);
            return;
        }
        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg[k], NULL , &dest_2d);
        LCDC_REP("%s: hal_dma_sg_2d_start ret=%d,dma_ch_mask=0x%x", __func__,ret,dma_ch_mask);
    }
#else
    uint32_t dma_line = HAL_DMA_MAX_DESC_XFER_SIZE/src_hight;
    uint32_t dma_frame_sz = dma_line*src_hight;

    uint32_t len = src_width*src_hight/DMA_COUNT;
    uint32_t desc_cnt = (len + dma_frame_sz - 1) / dma_frame_sz;
    if (desc_cnt > 128) {
        ASSERT(false, "%s: alloc memeory(size:%d) fail", __func__,
               sizeof(struct HAL_DMA_DESC_T) * desc_cnt);
    }
    if (dma_frame_sz > src_width*src_hight/DMA_COUNT)
        dma_frame_sz = src_width*src_hight/DMA_COUNT;

    LCDC_REP("%s: src_width :%d src_hight :%d", __func__,src_width,src_hight);
    LCDC_INFO("%s: desc_cnt=%d,src_ptr=0x%x,dst_ptr=0x%x", __func__,desc_cnt,src_ptr,dst_ptr);

#ifdef ROTATE_270
    struct HAL_DMA_2D_CFG_T src_2d;
    src_2d.xmodify = + src_width-1;
    src_2d.xcount = src_hight;
    src_2d.ymodify = -(src_width)*(src_hight) + src_width -2;
    src_2d.ycount = src_width/DMA_COUNT;
#else
    struct HAL_DMA_2D_CFG_T src_2d;
    src_2d.xmodify = - src_width-1;
    src_2d.xcount = src_hight;
    src_2d.ymodify = (src_width)*(src_hight) - src_width;
    src_2d.ycount = src_width/DMA_COUNT;
#endif

    start_tick = hal_fast_sys_timer_get();
    for (int k=0; k < DMA_COUNT; k++)
    {
        dma_ch[k] = hal_gpdma_get_chan(HAL_GPDMA_MEM, HAL_DMA_HIGH_PRIO);
        if (dma_ch[k] == HAL_DMA_CHAN_NONE) {
            LCDC_ERR("%s %d, ch[%d] eror", __func__, __LINE__,k);
            return;
        }
        dma_ch_mask |= (1<<dma_ch[k]);

        dma_cfg[k].handler = dma_irq_handler_covert;
        dma_cfg[k].dst_bsize = HAL_DMA_BSIZE_256;
        dma_cfg[k].dst_periph = HAL_GPDMA_MEM;
        dma_cfg[k].dst_width = FB_PIX_BYTE/2;
        dma_cfg[k].src_bsize = HAL_DMA_BSIZE_1;
        dma_cfg[k].src_periph = HAL_GPDMA_MEM;
        dma_cfg[k].src_width = FB_PIX_BYTE/2;
        dma_cfg[k].type = HAL_DMA_FLOW_M2M_DMA;
        dma_cfg[k].try_burst = 1;
        dma_cfg[k].ch = dma_ch[k];
        gra_sem_init(&covert_done[k], 0, 0);
        LCDC_INFO("%s: dma_ch=%d", __func__,dma_ch[k]);

        struct HAL_DMA_DESC_T *desc = &desc_bank[k][0];
        dma_cfg[k].dst = dst_ptr + k*src_width*FB_PIX_BYTE*src_hight/DMA_COUNT;
        dma_cfg[k].src_tsize = dma_frame_sz;
        int i = 0;
        for (i = 0; i < desc_cnt - 1; i++) {
#ifdef ROTATE_270
            dma_cfg[k].src =  src_ptr +  (src_width -1 - k*src_width/DMA_COUNT - i*dma_line)*FB_PIX_BYTE;
#else
            dma_cfg[k].src =  src_ptr +  (src_width*src_hight - src_width + k*src_width/DMA_COUNT + i*dma_line)*FB_PIX_BYTE;
#endif
            LCDC_INFO("%s: dmalink :%d dst=0x%x", __func__,i,dma_cfg[k].dst);
            ret =
                hal_dma_init_desc(&desc[i], &dma_cfg[k], &desc[i + 1], 0);
            if (ret != HAL_DMA_OK) {
                LCDC_ERR("%s:1 ret=%d", __func__,ret);
                return;
            }
            dma_cfg[k].dst += dma_frame_sz*FB_PIX_BYTE;
        }
#ifdef ROTATE_270
        dma_cfg[k].src =  src_ptr +  (src_width -1 - k*src_width/DMA_COUNT - i*dma_line)*FB_PIX_BYTE;
#else
        dma_cfg[k].src =  src_ptr +  (src_width*src_hight - src_width + k*src_width/DMA_COUNT + i*dma_line)*FB_PIX_BYTE;
#endif
        dma_cfg[k].src_tsize = (dst_ptr+(k+1)*src_width*FB_PIX_BYTE*src_hight/DMA_COUNT - dma_cfg[k].dst)/FB_PIX_BYTE;
        LCDC_INFO("%s: dmalink :%d dst=0x%x", __func__,i,dma_cfg[k].dst);
        ret = hal_dma_init_desc(&desc[i], &dma_cfg[k], NULL, 1);
        if (ret != HAL_DMA_OK) {
            LCDC_ERR("%s:2 ret=%d", __func__,ret);
            return;
        }
        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg[k], &src_2d, NULL);
        LCDC_REP("%s: hal_dma_sg_2d_start ret=%d,dma_ch_mask=0x%x", __func__,ret,dma_ch_mask);
    }
#endif
#if 0
#if defined(CONFIG_ROTATE_DMA)
    FAR struct lcdcdev_s *priv = &g_vtable;
    if (!ROTATE_USE_CPU(priv->pinfo.fbmem))
    {
        LCDC_REP("%s: hal_dma_sg_2d_start dma_ch_mask=0x%x", __func__,dma_ch_mask);
    }
    else
#endif
    {
        hw_wait_complete();
        LCDC_INFO("dma done use  %" PRIu32 " us\n",
                  FAST_TICKS_TO_US(hal_fast_sys_timer_get()-start_tick));
    }
#endif
}

#ifdef ROTATE_270
#define COPY_PIX(dst,src,src_width) do { *dst++ = *src; src += src_width;} while(0);
#else
#define COPY_PIX(dst,src,src_width) do { *dst++ = *src; src -= src_width;} while(0);
#endif

void cpu_rotate_90(uint32_t src_ptr, uint32_t dst_ptr,uint32_t src_width, uint32_t src_height)
{
    FB_PIX_TYPE *color_p = (FB_PIX_TYPE *)src_ptr;
    FB_PIX_TYPE *target = (FB_PIX_TYPE *)dst_ptr;
    start_tick = hal_fast_sys_timer_get();
    LCDC_REP("%s: src_width :%d src_hight :%d", __func__,src_width,src_height);
#ifdef ROTATE_270
    uint32_t pitch = src_width-1;
#else
    uint32_t pitch = (src_height-1)*src_width;
#endif
    for (int i=0; i< src_width;i++)
     {
        FB_PIX_TYPE *src = color_p+pitch-i;
        for (int j=src_height-1; j>=0;)
        {
            COPY_PIX(target, src, src_width);
            COPY_PIX(target, src, src_width);
#if BOARD_LCDC_WIDTH == 454
            j=j-2;
#else
            COPY_PIX(target, src, src_width);
            COPY_PIX(target, src, src_width);
            COPY_PIX(target, src, src_width);
            COPY_PIX(target, src, src_width);
#if BOARD_LCDC_WIDTH == 318
            j=j-6;
#else
            COPY_PIX(target, src, src_width);
            COPY_PIX(target, src, src_width);
            j=j-8;
#endif
#endif
        }
    }
    uint32_t lk = int_lock();
    lcdc_cache_sync((uint32_t)dst_ptr, src_width*FB_PIX_BYTE*src_height);
    int_unlock(lk);
    LCDC_REP("Commit buffer = %p -> %p  %" PRIu32 " us\n",
              (void *)color_p,(void *)dst_ptr,
              FAST_TICKS_TO_US(hal_fast_sys_timer_get()-start_tick));
}

void cpu_rotate_area_90(uint32_t src_ptr, uint32_t dst_ptr,int startx, int starty,uint32_t src_width, uint32_t src_height)
{
    start_tick = hal_fast_sys_timer_get();
    LCDC_REP("%s: (%d,%d) src_width :%d src_hight :%d", __func__,startx,starty,src_width,src_height);
#ifdef ROTATE_270
    uint32_t pitch = src_width-1;
#else
    uint32_t pitch = (src_height-1)*src_width;
#endif
    uint32_t endx,endy;
    endx = startx+src_width - 1;
#if BOARD_LCDC_WIDTH == 454
    starty = (starty)/2*2;
    src_height = (src_height +1)/2*2;
#elif BOARD_LCDC_WIDTH == 318
    starty = (starty)/6*6;
    src_height = (src_height +5)/6*6;
#else
    starty = (starty)/8*8;
    src_height = (src_height +7)/8*8;
#endif
    endy = starty+src_height - 1;

    if (endy+1 > BOARD_LCDC_WIDTH)
        endy = BOARD_LCDC_WIDTH -1;

    src_height = endy - starty +1;
    FB_PIX_TYPE *color_p = (FB_PIX_TYPE *)(src_ptr+(starty*BOARD_LCDC_HEIGHT+startx)*FB_PIX_BYTE);
    int dst_startx = starty;
    int dst_starty = BOARD_LCDC_HEIGHT-endx-1;
    FB_PIX_TYPE *target = (FB_PIX_TYPE *)(dst_ptr+(dst_starty*BOARD_LCDC_WIDTH+dst_startx)*FB_PIX_BYTE);
    //LCDC_REP("%s: (%d,%d)(%d,%d)(w:%d h:%d),%p",__func__,startx,starty,endx,endy,src_width,src_height,color_p);
    //LCDC_REP("%s: (%d,%d),%p",__func__,dst_startx,dst_starty,target);
    uint32_t x_count = 0;

    for (int i=startx; i<= endx;i++)
     {
        FB_PIX_TYPE *src = color_p+pitch-x_count++;
        //LCDC_REP("%s:x_count:%d i:%d,%p->%p",__func__,x_count,i,src,target);
        for (int j=endy; j>=starty;)
        {
            //if (check_buf_overwrite(target) != 0)
            //    LCDC_REP("%s:x_count:%d (%d,%d),%p->%p",__func__,x_count,i,j,src,target);
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
#if BOARD_LCDC_WIDTH == 454
            j=j-2;
#else
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
#if BOARD_LCDC_WIDTH == 318
            j=j-6;
#else
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
            COPY_PIX(target, src, BOARD_LCDC_HEIGHT);
            j=j-8;
#endif
#endif
        }
        target += BOARD_LCDC_WIDTH - src_height;
    }
    uint32_t lk = int_lock();
    lcdc_cache_sync((uint32_t)dst_ptr + dst_starty * BOARD_LCDC_WIDTH *FB_PIX_BYTE, src_width * BOARD_LCDC_WIDTH * FB_PIX_BYTE);
    //lcdc_cache_sync((uint32_t)dst_ptr, BOARD_LCDC_HEIGHT * BOARD_LCDC_WIDTH * FB_PIX_BYTE);
    int_unlock(lk);

    LCDC_REP("Commit buffer = %p -> %p  %" PRIu32 " us\n",
              (void *)color_p,(void *)dst_ptr,
              FAST_TICKS_TO_US(hal_fast_sys_timer_get()-start_tick));
}

