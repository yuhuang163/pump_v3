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

/**
 *
 * IO blocks:
 *
 * pngfiledata --> |pngdec| ---> pixeldata
 *                        |
 *                        ---> dma2d layer ---> |dma2d|--->
 *
 *
*/

#include "png_dec.h"
#include "cmsis_nvic.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_png.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_psram.h"
#include "hal_dma2d.h"
#include "stdio.h"
#include "string.h"
#include "hal_location.h"
#include "gra_wakelock.h"
#include "png_sem.h"
#include "check_raw_image.h"

#define PNGDEC_DRV_WITH_CACHEOPS

#define  GRAPHIC_DRV_DEBUG
#ifdef  GRAPHIC_DRV_DEBUG
#define PNGDEC_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define PNGDEC_REG(...) TR_INFO(TR_MOD(TEST), __VA_ARGS__)

#else
#define PNGDEC_INFO(...)
#define PNGDEC_REG(...)

#endif

#define PNGDEC_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define PNGDEC_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)

/**
 * Pre-processor Definitions
 */
#define PNG_BASE      SYS_PNG_BASE
#ifdef GRAPHICDRV_TIME_STATIS

#define _TEST_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)

#define TC_INIT  volatile uint32_t time_s; \
                 volatile uint32_t time_e;
#define TC_START time_s = hal_sys_timer_get();
#define TC_END   time_e = hal_sys_timer_get();

#define _TSTR(s) #s

#define TC_REP(s) do{ \
                      _TEST_REP(_TSTR(s)":%d us", TICKS_TO_US(time_e - time_s)); \
                    }while(0)

#else

#define TC_INIT
#define TC_START
#define TC_END
#define TC_REP(s)

#endif



#define FAR

/* Only 8 bit per pixel overal supported */

#define PNGDEC_PF_BYPP(n) ((n) / 8)

/**
 * Private Types
 */

/**
 * struct pngdec_s  -  PNGDEC driver interface
 * @pngdec     :   Public pngdec interface
 * @clut      :   Color lookup table
 * @dither    :   Color dither table
 */

struct pngdec_s {
    struct pngdec pngdec;
    /*private: internal use only */
    /** @lock : Ensure mutually exclusive access */
    struct sem *lock;
};

/**
 * struct pngdec_interrupt_s   - Interrupt handling
 * @irq           : irq number
 * @error         : error status
 * @handler       : handler entry
 */

struct pngdec_interrupt_s {
    int irq;        /* irq number */
    int error;        /* Interrupt error */
    int (*handler) (int irq, void *context, FAR void *arg);
    /** @sem : Semaphore for waiting for irq */
    struct sem *sem;
};

/**
 * Private Function Prototypes
 */


/**
 * Private Data
 */

static bool g_initialized = false;
static struct pngdec_interrupt_s g_interrupt;


/* The PNGDEC semaphore that enforces mutually exclusive access */

static struct sem g_lock;

/* Semaphore for interrupt handling */

static struct sem g_semirq;

/**
 * struct pngdec_s g_pngdecdev  - pngdec driver instance
 */

static struct pngdec_s g_pngdecdev = {
    .pngdec = {
        .decoder.base_addr = PNG_BASE,
    },
    .lock = &g_lock,
};


/**
 * Private PNGDEC Functions
 */

/**pngdec_cache_sync - Flush cache to external memory
 * @id : cache ID
 * @start_address  buffer start address
 * @len: buffer length
 *
 */
uint8_t pngdec_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
#ifndef PNGDEC_DRV_WITH_CACHEOPS
    return 0;
#endif
    uint32_t start_address_cache = start_address;
    uint32_t len_cache = len;
    //TRACE(3, "%s s:0x%x l:0x%x", __func__, start_address_cache, len_cache );
    return hal_cache_sync(id, start_address_cache, len_cache);
}

/**pngdec_cache_invalidate - sync external memory to cpu
 * @id : cache ID
 * @start_address  buffer start address
 * @len: buffer length
 *
 */
uint8_t pngdec_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
#ifndef PNGDEC_DRV_WITH_CACHEOPS
    return 0;
#endif
    uint32_t start_address_cache = start_address;
    uint32_t len_cache = len;
    //TRACE(3, "%s s:0x%x l:0x%x", __func__, start_address_cache, len_cache );
    return hal_cache_invalidate(id, start_address_cache, len_cache);
}

void set_pngdec_done_interrupt_handler(struct pngdec_interrupt_s *irq)
{
    NVIC_SetVector(irq->irq, (uint32_t) irq->handler);
    NVIC_SetPriority(irq->irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(irq->irq);
    NVIC_EnableIRQ(irq->irq);
}

volatile bool png_to_dma2d = false;
/**
 * pngdecirq -PNGDEC interrupt handler
 *
 * Returns: OK On success, -errno on failure.
 */

static int pngdec_irq(int irq, void *context, FAR void *arg)
{
    int ret;
    struct png_decoder * dec = &g_pngdecdev.pngdec.decoder;
    FAR struct pngdec_interrupt_s *priv = &g_interrupt;
    ret = hal_png_decoder_irq_err(dec);
    PNGDEC_INFO("pngdec irq = %d, err = %08x\n", irq, ret);
    if (ret) {
        /* Configuration error interrupt */
        PNGDEC_REP("PNGDEC ERROR: \n");
        priv->error = ret;
    }else {
        /* complete interrupt */
        priv->error = 0;
    }
    hal_png_decoder_irq_clear(dec);
    /* Unlock the semaphore if locked */
    if (png_to_dma2d == true) {
        return 0;
    }
    ret = pngdec_sem_post(priv->sem);
    if (ret < 0) {
        PNGDEC_ERR("ERROR: pngdec_sem_post() failed\n");
    }
    return priv->error;
}

/**
 * pngdec_waitforirq -Helper waits until the pngdec irq occurs.
 *
 * hat means that an ongoing clut.
 * loading or dma transfer was completed.
 * Note! The caller must use this function within a critical section.
 *
 * Returns: OK On success, -errno on failure.
 */

static int pngdec_waitforirq(void)
{
    int ret;
    FAR struct pngdec_interrupt_s *priv = &g_interrupt;
    if (png_to_dma2d == true) {
       return 0;
    }
    TC_INIT
    PNGDEC_REG("pngdec_waitforirq();");
    TC_START
    ret = pngdec_sem_wait(priv->sem);
    if (ret < 0) {
        PNGDEC_ERR("ERROR: pngdec_sem_wait() failed\n");
        TC_END
        TC_REP(png_cost);
        return ret;
    }
    ret = priv->error;
    TC_END
    TC_REP(png_cost);
    return ret;
}

void hal_cmu_pngdec_sleep(void);
void hal_cmu_pngdec_wakeup(void);

static int pngdec_start(void)
{
    struct png_decoder * dec = &g_pngdecdev.pngdec.decoder;
    int ret = 0;
    //gra_exit_sleep(40);
    hal_png_decoder_start(dec);
    ret = pngdec_waitforirq();
    return ret;
}

static int pngdec_pixdma_start(struct png_pix_buffer* dest)
{
    struct png_decoder * dec = &g_pngdecdev.pngdec.decoder;
    int ret = 0;
    //gra_exit_sleep(40);
    TRACE(0,"%s %d",__func__,__LINE__);
    if (dest->dma2dfb) {
        TRACE(0,"%s %d png enable 2d stop pixdma",__func__,__LINE__);
        hal_png_decoder_dma2d_start(dec);
        hal_png_decoder_pixdma_stop(dec);
    } else {
        hal_png_decoder_pixdma_start(dec);
        hal_png_decoder_dma2d_stop(dec);
    }
    TRACE(0,"%s %d before png wait",__func__,__LINE__);
    ret = pngdec_waitforirq();
    TRACE(0,"%s %d before 2d start",__func__,__LINE__);
    dest->dma2d_start();
    TRACE(0,"%s %d frame done",__func__,__LINE__);
    return ret;
}

static bool is_invalid_compress_mode(uint8_t c_mode)
{
    bool ret = false;
    ret = ((c_mode & 0x7) != 0x00); //not 1. use defalte to Huffman
                                    //    2. 1 byte 5 filter modes
                                    //    3. not interlace line scan mode
    return ret;
}

static bool is_invalid_color_type(enum PNG_COLOR_TYPE_T type)
{
    bool ret = false;
    ret = /*(type != PNG_COLOR_GRAY) && */ (type != PNG_COLOR_RGB888) &&
          (type != PNG_COLOR_IRGB888) /*&& (type != PNG_COLOR_GRAYA)*/ &&
          (type != PNG_COLOR_RGBA888);
    return ret;
}

static bool is_invalid_image_size(uint16_t width, uint16_t height)
{
    return (width > 512);
}

static enum PNG_PIX_FMT_T get_png_fmt(struct png_pix_buffer* dest, enum PNG_COLOR_TYPE_T type)
{
   enum PNG_PIX_FMT_T fmt = PNG_PIX_FMT_RGB888;
   switch(type) {
       case PNG_COLOR_RGB888:
       case PNG_COLOR_IRGB888:
           if (dest->fmt == PNGDEC_PIX_FMT_RGB565 ) {
              fmt = PNG_PIX_FMT_RGB565;
           }
           if (dest->fmt == PNGDEC_PIX_FMT_ARGB565 ) {
              fmt = PNG_PIX_FMT_RGB888;
           }
           if (dest->fmt == PNGDEC_PIX_FMT_RGBA565 ) {
              fmt = PNG_PIX_FMT_RGB888;
           }
           if (dest->fmt == PNGDEC_PIX_FMT_RGB888 ) {
              fmt = PNG_PIX_FMT_RGB888;
           }
           break;

       case PNG_COLOR_RGBA888:
           fmt = PNG_PIX_FMT_RGBA888;
           if (dest->fmt == PNGDEC_PIX_FMT_RGB565 ) {
              fmt = PNG_PIX_FMT_RGB565;
           }
           if (dest->fmt == PNGDEC_PIX_FMT_ARGB565 ) {
              fmt = PNG_PIX_FMT_RGB888;
           }
           if (dest->fmt == PNGDEC_PIX_FMT_RGBA565 ) {
              fmt = PNG_PIX_FMT_RGB888;
           }
           if (dest->fmt == PNGDEC_PIX_FMT_RGB888 ) {
              fmt = PNG_PIX_FMT_RGB888;
           }
           if (dest->fmt == PNGDEC_PIX_FMT_ARGB888 ) {
              fmt = PNG_PIX_FMT_ARGB888;
           }
           break;
       default:
           fmt = PNG_PIX_FMT_RGB888;
    }
    return fmt;
}

/*
 * public api
 */

/**
 * pngdec -   decode png file int image pixes
 * @pngfile : png file data buffer
 * @dest    : png pixes data buffer
 *
 * Returns: OK on success ,-errno on failure.
 */
int pngdec(struct png_file_data* pngfile, struct png_pix_buffer* dest)
{
    int ret = 0;
    struct png_decoder * dec = &g_pngdecdev.pngdec.decoder;
    FAR struct pngdec_interrupt_s *priv = &g_interrupt;
    uint16_t pitch = 0;
    struct png_imginfo info;
    uint32_t lines = 0;
    uint32_t *pixbuffer;
    enum PNG_PIX_FMT_T  fmt = 0;
    pngdec_initialize();
    ASSERT(pngfile != NULL && dest != NULL, "%s",
           __func__);
    pngdec_lock_wait(&g_lock);
    hal_png_decoder_clk_enable(dec);
    hal_png_decoder_file_size(dec,pngfile->size);
    hal_png_decoder_png_file_start_address(dec, pngfile->data);
    hal_png_decoder_input_burst_length(dec,128);
    pngdec_cache_sync( HAL_CACHE_ID_D_CACHE,pngfile->data, pngfile->size );
    /* Start PNGDEC to parse file header and wait until completed */
    ret = pngdec_start();
    /*get png image info parse result*/
    hal_png_decoder_get_imginfo(dec, &info);
    if (is_invalid_compress_mode(info.c_mode)) {
       ret = INVALID_COMPRESS_MODE;
    }
    if (is_invalid_color_type(info.type)) {
       ret = INVALID_COLOR_TYPE;
    }
    if (is_invalid_image_size(info.width, info.height)) {
       ret = INVALID_IMAGE_SIZE;
    }
    if (ret){
        PNGDEC_ERR("%s file header err:0x%x \n", __func__, ret);
        pngdec_sem_post(&g_lock);
        return ret;
    }
    /*set pixdma*/
    lines = info.height;
    hal_png_decoder_output_pix_rgb565(dec, false);
    hal_png_decoder_output_pix_argb565(dec, false);
    hal_png_decoder_output_pix_rgba565(dec, false);

    if (info.bpp != 8) {
       ret = -1;
       goto  __eixt_handler;
    }
    if (dest->fmt == PNGDEC_PIX_FMT_RGBA565 ||dest->fmt == PNGDEC_PIX_FMT_ARGB565 ||
     dest->fmt == PNGDEC_PIX_FMT_RGB565) {
        if ( dest->fmt == PNGDEC_PIX_FMT_RGBA565 ) {
           hal_png_decoder_output_pix_rgba565(dec, true);
           pitch = 3* info.width;
        }
        if ( dest->fmt == PNGDEC_PIX_FMT_ARGB565 ) {
           hal_png_decoder_output_pix_argb565(dec, true);
           pitch = 3* info.width;
        }
        if ( dest->fmt == PNGDEC_PIX_FMT_RGB565) {
           hal_png_decoder_output_pix_rgb565(dec, true);
           pitch = 2* info.width;
        }
    } else if ((dest->fmt == PNGDEC_PIX_FMT_RGB888 ||dest->fmt == PNGDEC_PIX_FMT_RGBA888 ||
     dest->fmt == PNGDEC_PIX_FMT_ARGB888)) {
         if (dest->fmt == PNGDEC_PIX_FMT_RGB888) {
             pitch = 3* info.width;
         } else {
             pitch = 4* info.width;
         }
    } else {
       ret = -2;
       goto  __eixt_handler;
    }
    dest->stride = pitch;
    dest->width = info.width;
    dest->height = info.height;
    pixbuffer = dest->pixbufalloc(pitch, 8);
#if 1
    if (dest->pitch > pitch) {
        pitch = dest->pitch;
    } else {
        dest->pitch = pitch;
    }
#endif
    PNGDEC_REP("%s stride:%d w:%d h:%d destbuffer:%p ",__func__,
               dest->stride, dest->width, dest->height, pixbuffer);
    if ( dest->globle_alpha ){
        hal_png_decoder_ena_output_globle_alpha(dec,true);
        hal_png_decoder_set_output_globle_alpha(dec, dest->alpha);
    } else {
        hal_png_decoder_ena_output_globle_alpha(dec,false);
    }
    if ( dest->trigger_lines ) {
        hal_png_decoder_pixdma_output_trigger_lines(dec, dest->trigger_lines);
    } else {
        hal_png_decoder_pixdma_output_trigger_lines(dec, info.height);
    }
    fmt = get_png_fmt(dest, info.type);
    hal_png_decoder_pix_color_format(dec,fmt);
    if (dest->dma2dfb) {
        TRACE(0,"%spngfb_enable lid:%d ",__func__,dest->dma2dfb->lid );
        hal_dma2d_pngfb_enable(dest->dma2dfb->lid, 1);
        png_to_dma2d = true;
    } else {
        hal_png_decoder_pixdma_start_address(dec,pixbuffer);
    }
    hal_png_decoder_image_pitch(dec, pitch);
    hal_png_decoder_pixdma_burst_length(dec,128);
    ret = pngdec_pixdma_start(dest);
    if (ret){
        PNGDEC_ERR("%s decoder err:0x%x \n", __func__, ret);
        ret = PNGDEC_DEC_ERROR;
    }
    if (png_to_dma2d == false) {
        if ( dest->trigger_lines ) {
             if(dest->trigger_lines_handler) {
                dest->trigger_lines_handler();
             }
             //wait second interrupt
             ret = pngdec_waitforirq();
             if (ret){
                 PNGDEC_ERR("%s decoder err:0x%x \n", __func__, ret);
                 ret = PNGDEC_DEC_ERROR;
             }
        }
    }
    hal_png_decoder_irqmask_clear(dec);
    __eixt_handler:
    hal_png_decoder_stop(dec);
    hal_png_decoder_pixdma_stop(dec);
    png_to_dma2d = false;
    pngdec_cache_invalidate(HAL_CACHE_ID_D_CACHE,pixbuffer,info.height*info.width*info.bpp/8);
    pngdec_sem_post(&g_lock);
    return ret;
}

/**
 * pngdec_initialize - Initialize the pngdec controller
 *
 * Initialize the pngdec controller
 *
 * Returns: OK - On success, an error if initializing failed.
 */

int pngdec_initialize(void)
{
    struct png_decoder * dec = &g_pngdecdev.pngdec.decoder;
    PNGDEC_INFO("Initialize PNGDEC driver\n");
    if (g_initialized == false) {
        hal_cmu_pngdec_clock_enable();
        g_interrupt.irq = hal_png_decoder_irqn(dec);
        pngdec_sem_init(&g_lock, 0, 1);
        g_interrupt.sem = &g_semirq;
        pngdec_sem_init(g_interrupt.sem, 0, 0);
        g_interrupt.handler = pngdec_irq;
        hal_png_decoder_irq_disable(dec);
        /* Attach PNGDEC interrupt vector */
        set_pngdec_done_interrupt_handler(&g_interrupt);
        hal_png_decoder_irq_enable(dec);
        /* Enable the IRQ at the NVIC */
        g_initialized = true;
    }
    return 0;
}

/**
 * pngdec_uninitialize - Uninitialize the pngdec controller
 *
 * Uninitialize the pngdec controller
 *
 */
void pngdec_uninitialize(void)
{
    struct png_decoder * dec = &g_pngdecdev.pngdec.decoder;
    /* Disable PNGDEC clk */
    hal_cmu_pngdec_clock_disable();
    g_initialized = false;
}