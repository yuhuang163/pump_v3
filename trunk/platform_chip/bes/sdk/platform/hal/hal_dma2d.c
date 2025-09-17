/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#if defined(IMGDMA_BASE)

#include "string.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "reg_dma2d.h"
#include "hal_dma2d.h"

//#define DMA2D_DEBUG
#ifdef  DMA2D_DEBUG
#define DMA2D_INFO(...)         TR_INFO( TR_MOD(HAL), "//DMA2D: " __VA_ARGS__)
#else
#define DMA2D_INFO(...)
#endif

#define DMA2D_ERR(...)          TR_ERROR( TR_MOD(HAL), "//DMA2D: " __VA_ARGS__)

#ifndef DMA2D_MHZ
#define DMA2D_MHZ               200
#endif

#ifdef CHIP_BEST1700
#define DMA2D_STBY_LEN          0x100
#else
#define DMA2D_STBY_LEN          0x900
#endif

#define DMA2D_ZOOM_MIN          25
#define DMA2D_ZOOM_MAX          400

/* buffer width need aligned to 16 pixels */
#define DMA2D_ALIGN_PIXELS      16
/* buffer memory address need aligned to 8 bytes */
#define DMA2D_ALIGN_BYTES       8

/* workaround for scale random hang-up issue */
#define DMA2D_ZOOM_WORKAROUND   1

static struct DMA2D_REG_T *dma2d = (struct DMA2D_REG_T *)IMGDMA_BASE;

static bool dma2d_working = false;

static void (*dma2d_done_cb)(int ret) = NULL;

static void hal_dma2d_irq_handler(void)
{
    int ret = 0;
    uint32_t val = dma2d->REG_1C4;
    DMA2D_INFO("irq status 0x%x", val);
    if (val & DMA2D_IRQ_LAYER_WB_FRAMEDONE) {
        if (dma2d_done_cb)
            dma2d_done_cb(ret);
        dma2d_working = false;
    }
}

int hal_dma2d_open(void (*done_cb)(int))
{
    hal_cmu_dma2d_clock_enable(DMA2D_MHZ);

    /* Enable PN/TV Layers */
    dma2d->REG_164 = DMA2D_TV_LAYERS_ENABLE | DMA2D_PN_LAYERS_ENABLE;

    dma2d_done_cb = done_cb;

    NVIC_SetVector(IMGDMA_IRQn, (uint32_t)hal_dma2d_irq_handler);
    NVIC_SetPriority(IMGDMA_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(IMGDMA_IRQn);
    NVIC_EnableIRQ(IMGDMA_IRQn);

    DMA2D_INFO("opened.");
    return DMA2D_OK;
}

int hal_dma2d_close(void)
{
    NVIC_ClearPendingIRQ(IMGDMA_IRQn);
    NVIC_DisableIRQ(IMGDMA_IRQn);

    dma2d->REG_160 = DMA2D_TV_LAYERS_DISABLE | DMA2D_PN_LAYERS_DISABLE;

    hal_cmu_dma2d_clock_disable();

    dma2d_working = false;
    dma2d_done_cb = NULL;
    DMA2D_INFO("closed.");
    return DMA2D_OK;
}

static int dma2d_get_bpp(enum DMA2D_COLOR_FORMAT_T format)
{
    switch (format) {
        case DMA2D_RGB565:
        case DMA2D_BGR565:
            return 2;
        case DMA2D_RGB888:
        case DMA2D_BGR888:
            return 3;
        case DMA2D_RGBA8888:
        case DMA2D_BGRA8888:
        case DMA2D_ARGB8888:
        case DMA2D_ABGR8888:
            return 4;
        default:
            return 1;
    }
    return 1;
}

static void dma2d_reset(void)
{
    /* Workaround: DMA2D writeback framedone irq is not reliable,
     * add some delays to wait the previous operation is really done.
     */
    hal_sys_timer_delay(2);

    hal_cmu_dma2d_reset_set();
    hal_sys_timer_delay(1);
    hal_cmu_dma2d_reset_clear();
}

static void dma2d_framedone_irq_enable(void)
{
    dma2d->REG_1C0 = DMA2D_IRQ_LAYER_WB_FRAMEDONE;
#ifdef DMA2D_DEBUG
    dma2d->REG_1C0 |= DMA2D_IRQ_LAYER_GRA_FRAMEDONE0 | DMA2D_IRQ_LAYER_GRA_FRAMEDONE1 |
                      DMA2D_IRQ_LAYER_DMA_FRAMEDONE0 | DMA2D_IRQ_LAYER_DMA_FRAMEDONE1 |
                      DMA2D_IRQ_LAYER_TVG_FRAMEDONE0 | DMA2D_IRQ_LAYER_TVG_FRAMEDONE1;
#endif
}

static void dma2d_config_dma_layer(int w, int h, int zoom_w, int zoom_h,
                                   int format, int pitch, uint32_t addr)
{
    dma2d->REG_190 = DMA2D_LAYER_DMA_ENABLE;
    if (format & 0x80)
        dma2d->REG_190 |= DMA2D_LAYER_DMA_SWAP_RB;
    switch (format) {
        case DMA2D_RGBA8888:
        case DMA2D_BGRA8888:
            dma2d->REG_190 |= DMA2D_LAYER_DMA_COLOR_MODE(0x4);
            break;
        case DMA2D_ARGB8888:
        case DMA2D_ABGR8888:
            dma2d->REG_190 |= DMA2D_LAYER_DMA_COLOR_MODE(0xb);
            break;
        case DMA2D_RGB888:
        case DMA2D_BGR888:
            dma2d->REG_190 |= DMA2D_LAYER_DMA_COLOR_MODE(0x2);
            break;
        case DMA2D_RGB565:
        case DMA2D_BGR565:
            dma2d->REG_190 |= DMA2D_LAYER_DMA_COLOR_MODE(0x0);
            break;
        default:
            break;
    }
    dma2d->REG_0E8 = DMA2D_LAYER_DMA_YPOS(1) | DMA2D_LAYER_DMA_XPOS(1);
    dma2d->REG_0EC = DMA2D_LAYER_DMA_WIDTH(w) | DMA2D_LAYER_DMA_HEIGHT(h);
    dma2d->REG_0F0 = DMA2D_LAYER_DMA_ZOOM_WIDTH(zoom_w) |
                     DMA2D_LAYER_DMA_ZOOM_HEIGHT(zoom_h);
    dma2d->REG_0E0 = DMA2D_LAYER_DMA_PITCH(pitch);
    dma2d->REG_0C0 = DMA2D_LAYER_DMA_ADDR(ADDR_CPU_TO_DEV(addr));
}

static void dma2d_config_gra_layer(int w, int h, int zoom_w, int zoom_h,
                                   int format, int pitch, uint32_t addr)
{
    dma2d->REG_264 = DMA2D_LAYER_GRA_ENABLE;
    if (format & 0x80)
        dma2d->REG_264 |= DMA2D_LAYER_GRA_SWAP_RB;
    switch (format) {
        case DMA2D_RGBA8888:
        case DMA2D_BGRA8888:
            dma2d->REG_264 |= DMA2D_LAYER_GRA_COLOR_MODE(0x4);
            break;
        case DMA2D_ARGB8888:
        case DMA2D_ABGR8888:
            dma2d->REG_264 |= DMA2D_LAYER_GRA_COLOR_MODE(0xb);
            break;
        case DMA2D_RGB888:
        case DMA2D_BGR888:
            dma2d->REG_264 |= DMA2D_LAYER_GRA_COLOR_MODE(0x2);
            break;
        case DMA2D_RGB565:
        case DMA2D_BGR565:
            dma2d->REG_264 |= DMA2D_LAYER_GRA_COLOR_MODE(0x0);
            break;
        default:
            break;
    }
    dma2d->REG_100 = DMA2D_LAYER_GRA_YPOS(1) | DMA2D_LAYER_GRA_XPOS(1);
    dma2d->REG_104 = DMA2D_LAYER_GRA_WIDTH(w) | DMA2D_LAYER_GRA_HEIGHT(h);
    dma2d->REG_108 = DMA2D_LAYER_GRA_ZOOM_WIDTH(zoom_w) |
                     DMA2D_LAYER_GRA_ZOOM_HEIGHT(zoom_h);
    dma2d->REG_0FC = DMA2D_LAYER_GRA_PITCH(pitch);
    dma2d->REG_0F4 = DMA2D_LAYER_GRA_ADDR(ADDR_CPU_TO_DEV(addr));
}

static void dma2d_config_tvg_layer(int w, int h, int zoom_w, int zoom_h,
                                   int format, int pitch, uint32_t addr)
{
    dma2d->REG_080 = DMA2D_LAYER_TVG_ENABLE;
    if (format & 0x80)
        dma2d->REG_080 |= DMA2D_LAYER_TVG_SWAP_RB;
    switch (format) {
        case DMA2D_RGBA8888:
        case DMA2D_BGRA8888:
            dma2d->REG_080 |= DMA2D_LAYER_TVG_COLOR_MODE(0x4);
            break;
        case DMA2D_ARGB8888:
        case DMA2D_ABGR8888:
            dma2d->REG_080 |= DMA2D_LAYER_TVG_COLOR_MODE(0xb);
            break;
        case DMA2D_RGB888:
        case DMA2D_BGR888:
            dma2d->REG_080 |= DMA2D_LAYER_TVG_COLOR_MODE(0x2);
            break;
        case DMA2D_RGB565:
        case DMA2D_BGR565:
            dma2d->REG_080 |= DMA2D_LAYER_TVG_COLOR_MODE(0x0);
            break;
        default:
            break;
    }
    dma2d->REG_040 = DMA2D_LAYER_TVG_YPOS(1) | DMA2D_LAYER_TVG_XPOS(1);
    dma2d->REG_044 = DMA2D_LAYER_TVG_WIDTH(w) | DMA2D_LAYER_TVG_HEIGHT(h);
    dma2d->REG_048 = DMA2D_LAYER_TVG_ZOOM_WIDTH(zoom_w) |
                     DMA2D_LAYER_TVG_ZOOM_HEIGHT(zoom_h);
    dma2d->REG_03C = DMA2D_LAYER_TVG_PITCH(pitch);
    dma2d->REG_034 = DMA2D_LAYER_TVG_ADDR(ADDR_CPU_TO_DEV(addr));
}

static void dma2d_config_out_layer(int w, int h, int format, int pitch, uint32_t addr)
{
    dma2d->REG_200 = DMA2D_LAYER_WB_ENABLE | DMA2D_LAYER_WB_PITCH(pitch);
    switch (format) {
        case DMA2D_RGBA8888:
        case DMA2D_BGRA8888:
            dma2d->REG_200 |= DMA2D_LAYER_WB_COLOR_MODE(0x2);
            break;
        case DMA2D_ARGB8888:
        case DMA2D_ABGR8888:
            dma2d->REG_200 |= DMA2D_LAYER_WB_COLOR_MODE(0x3);
            break;
        case DMA2D_RGB888:
        case DMA2D_BGR888:
            dma2d->REG_200 |= DMA2D_LAYER_WB_COLOR_MODE(0x1);
            break;
        case DMA2D_RGB565:
        case DMA2D_BGR565:
            dma2d->REG_200 |= DMA2D_LAYER_WB_COLOR_MODE(0x0);
            break;
        default:
            break;
    }
    dma2d->REG_204 = DMA2D_LAYER_WB_WIDTH(w) | DMA2D_LAYER_WB_HEIGHT(h) |
                     DMA2D_LAYER_WB_FRAME_PER_TOGGLE(1);
    dma2d->REG_208 = DMA2D_LAYER_WB_ADDR(ADDR_CPU_TO_DEV(addr));
}

static void dma2d_config_smart_panel(int w, int h, int format, uint32_t color)
{
    dma2d->REG_188 = DMA2D_SPU_ENABLE;
    if (format == DMA2D_RGB565 || format == DMA2D_BGR565)
        dma2d->REG_188 |= DMA2D_SPU_SMPN_MODE(5);
    else
        dma2d->REG_188 |= DMA2D_SPU_SMPN_MODE(3);
    dma2d->REG_118 = DMA2D_SPU_WIDTH(w + 1) | DMA2D_SPU_HEIGHT(h + 1);
    dma2d->REG_114 = DMA2D_SPU_HTOTAL(w + 2) | DMA2D_SPU_VTOTAL(h + 2);
    dma2d->REG_11C = DMA2D_SPU_HFPORCH(12) | DMA2D_SPU_HBPORCH(10);
    dma2d->REG_120 = DMA2D_SPU_VFPORCH(10) | DMA2D_SPU_VBPORCH(10);
    dma2d->REG_124 = DMA2D_SPU_BGCOLOR(color);
}

static void dma2d_config_blend(int mode, uint8_t alpha1, uint8_t alpha2)
{
    dma2d->REG_194 = DMA2D_BLEND_DMA_TRANS_TRIGGER;
    if (mode == DMA2D_BLEND_CONST_ALPHA) {
        dma2d->REG_194 |= DMA2D_BLEND_PN_ALPHA_VAL(alpha1);
        dma2d->REG_084 = DMA2D_BLEND_TV_ALPHA_VAL(alpha2);
        dma2d->REG_1E8 = DMA2D_BLEND_MODE(0);
    } else if (mode == DMA2D_BLEND_PIXEL_ALPHA) {
        dma2d->REG_194 |= DMA2D_BLEND_PN_PIXEL_MODE;
        dma2d->REG_194 |= DMA2D_BLEND_TV_PIXEL_MODE;
        dma2d->REG_1E8 = DMA2D_BLEND_MODE(0);
    } else {
        dma2d->REG_194 |= DMA2D_BLEND_PN_ALPHA_VAL(0xff);
        dma2d->REG_084 = DMA2D_BLEND_TV_ALPHA_VAL(0xff);
        dma2d->REG_1E8 = DMA2D_BLEND_MODE(0);
    }
}

static void dma2d_config_process_path(void)
{
    dma2d->REG_1DC = DMA2D_VIDEO_PATH_AUTO_SET;
    //dma2d->REG_1DC = DMA2D_VIDEO_PATH_MANUAL_SET | DMA2D_VIDEO_PATH_SEL(0);
}

static void dma2d_config_vsync(int stby_len)
{
    /* enable dispmem and disable vsync trigger */
    dma2d->REG_210 |= DMA2D_DISP_MEM_ENABLE | DMA2D_DISP_SVSYNC_TRIG_DISABLE |
                     DMA2D_DISP_AVSYNC_TRIG_DISABLE;

    dma2d->REG_21C = DMA2D_SMPN_HFPORCH(0x20) | DMA2D_SMPN_HBPORCH(0x20) |
                     DMA2D_VSYNC_LINE_NUM(0xa);
    dma2d->REG_220 = DMA2D_VSYNC_START_PIX(0x1e) | DMA2D_VSYNC_END_PIX(0x1f);
    dma2d->REG_218 = DMA2D_VSYNC_START_LINE(1) | DMA2D_VSYNC_END_LINE(5) |
                     DMA2D_VSYNC_STANDBY_LEN(stby_len);
}

static void dma2d_config_burst_size(void)
{
    dma2d->REG_200 |= DMA2D_LAYER_WB_BURST_LEN(8);
    dma2d->REG_1DC |= DMA2D_LAYER_DMA_BURST_LEN(3) | DMA2D_LAYER_GRA_BURST_LEN(3);
}

static void dma2d_frame_start(void)
{
    dma2d->REG_260 |= DMA2D_AXIBUS_FAST_MODE;
    dma2d->REG_224 = DMA2D_FRAME_START;
}

int hal_dma2d_clear(struct DMA2D_BUFFER_T *buf, struct DMA2D_AREA_T *area, uint32_t color)
{
    int pitch = 0;
    int dst_w = 0;
    int dst_h = 0;
    uint32_t addr;

    if (!buf) {
        DMA2D_ERR("clear: target buffer is NULL.");
        return DMA2D_FAILED;
    }
    if (!buf->data) {
        DMA2D_ERR("clear: data address is NULL.");
        return DMA2D_FAILED;
    }
    if (buf->width == 0 || buf->height == 0) {
        DMA2D_ERR("clear: buffer size cannot be zero.");
        return DMA2D_FAILED;
    }
    if (buf->width % DMA2D_ALIGN_PIXELS) {
        DMA2D_ERR("clear: buffer width need align to %d pixels.", DMA2D_ALIGN_PIXELS);
        return DMA2D_UNALIGNMENT;
    }
    if (((uint32_t)buf->data) & (DMA2D_ALIGN_BYTES - 1)) {
        DMA2D_ERR("clear: buffer address need align to %d bytes.", DMA2D_ALIGN_BYTES);
        return DMA2D_UNALIGNMENT;
    }

    pitch = buf->width * dma2d_get_bpp(buf->format);

    if (area) {
        if (area->x < 0 || area->x >= buf->width ||
            area->y < 0 || area->y >= buf->height) {
            DMA2D_ERR("clear: invalid target area.");
            return DMA2D_FAILED;
        }

        if (area->x + area->w > buf->width)
            dst_w = buf->width - area->x;
        else
            dst_w = area->w;

        if (area->y + area->h > buf->height)
            dst_h = buf->height - area->y;
        else
            dst_h = area->h;

        addr = (uint32_t)buf->data + area->y * pitch + area->x * dma2d_get_bpp(buf->format);
    } else {
        dst_w = buf->width;
        dst_h = buf->height;
        addr = (uint32_t)buf->data;
    }

    if (dst_w <= 0 || dst_h <= 0) {
        DMA2D_INFO("clear: target area is zero, do nothing.");
        return DMA2D_OK;
    }

    /* force memory address aligned */
    addr &= ~(DMA2D_ALIGN_BYTES - 1);

    /* reset dma2d for next work */
    dma2d_reset();

    /* framedone irq enable */
    dma2d_framedone_irq_enable();

    /* Write color to first pixel */
    memcpy((void *)addr, &color, dma2d_get_bpp(buf->format));

    /* config layers */
    dma2d_config_gra_layer(1, 1, dst_w, dst_h, buf->format, pitch, addr);
    dma2d_config_out_layer(dst_w, dst_h, buf->format, pitch, addr);

    /* config smart panel */
    dma2d_config_smart_panel(dst_w, dst_h, buf->format, color);

    dma2d_config_blend(DMA2D_BLEND_NONE, 0, 0);
    dma2d_config_process_path();
    dma2d_config_vsync(DMA2D_STBY_LEN);
    dma2d_config_burst_size();
    dma2d_frame_start();

    dma2d_working = true;
    DMA2D_INFO("clear: start fill buffer 0x%x by color 0x%x, area %dx%d.",
                addr, color, dst_w, dst_h);
    return DMA2D_OK;
}

int hal_dma2d_blend(struct DMA2D_BUFFER_T *src_buf, struct DMA2D_AREA_T *src_area,
                    struct DMA2D_BUFFER_T *dst_buf, struct DMA2D_AREA_T *dst_area,
                    enum DMA2D_BLEND_MODE_T blend, uint8_t alpha,
                    int scale_x, int scale_y)
{
    int src_pitch = 0;
    int dst_pitch = 0;
    int src_w = 0;
    int src_h = 0;
    int zoom_w = 0;
    int zoom_h = 0;
    int dst_w = 0;
    int dst_h = 0;
    uint32_t src_addr;
    uint32_t dst_addr;

    if (!src_buf || !dst_buf) {
        DMA2D_ERR("blend: source or target dma2d buffer is NULL.");
        return DMA2D_FAILED;
    }
    if (!src_buf->data || !dst_buf->data) {
        DMA2D_ERR("blend: source or target data address is NULL.");
        return DMA2D_FAILED;
    }
    if (scale_x < DMA2D_ZOOM_MIN || scale_x > DMA2D_ZOOM_MAX ||
        scale_y < DMA2D_ZOOM_MIN || scale_y > DMA2D_ZOOM_MAX) {
        DMA2D_ERR("blend: scale percent must in 25%% ~ 400%%.");
        return DMA2D_FAILED;
    }
    if (src_buf->width == 0 || src_buf->height == 0 ||
        dst_buf->width == 0 || dst_buf->height == 0) {
        DMA2D_ERR("blend: buffer size cannot be zero.");
        return DMA2D_FAILED;
    }
    if (src_buf->width % DMA2D_ALIGN_PIXELS || dst_buf->width % DMA2D_ALIGN_PIXELS) {
        DMA2D_ERR("blend: buffer width need align to %d pixels.", DMA2D_ALIGN_PIXELS);
        return DMA2D_UNALIGNMENT;
    }
    if (((uint32_t)src_buf->data) & (DMA2D_ALIGN_BYTES - 1) ||
        ((uint32_t)dst_buf->data) & (DMA2D_ALIGN_BYTES - 1)) {
        DMA2D_ERR("blend: buffer address need align to %d bytes.", DMA2D_ALIGN_BYTES);
        return DMA2D_UNALIGNMENT;
    }

    src_pitch = src_buf->width * dma2d_get_bpp(src_buf->format);
    dst_pitch = dst_buf->width * dma2d_get_bpp(dst_buf->format);

    if (src_area) {
        if (src_area->x < 0 || src_area->x >= src_buf->width ||
            src_area->y < 0 || src_area->y >= src_buf->height) {
            DMA2D_ERR("blend: invalid source area [%d,%d,%d,%d]",
                       src_area->x, src_area->y, src_area->w, src_area->h);
            return DMA2D_FAILED;
        }

        if (src_area->x + src_area->w > src_buf->width)
            src_w = src_buf->width - src_area->x;
        else
            src_w = src_area->w;

        if (src_area->y + src_area->h > src_buf->height)
            src_h = src_buf->height - src_area->y;
        else
            src_h = src_area->h;

        src_addr = (uint32_t)src_buf->data + src_area->y * src_pitch
                    + src_area->x * dma2d_get_bpp(src_buf->format);
    } else {
        src_w = src_buf->width;
        src_h = src_buf->height;
        src_addr = (uint32_t)src_buf->data;
    }

    if (dst_area) {
        if (dst_area->x < 0 || dst_area->x >= dst_buf->width ||
            dst_area->y < 0 || dst_area->y >= dst_buf->height) {
            DMA2D_ERR("blend: invalid target area.");
            return DMA2D_FAILED;
        }

        if (dst_area->x + dst_area->w > dst_buf->width)
            dst_w = dst_buf->width - dst_area->x;
        else
            dst_w = dst_area->w;

        if (dst_area->y + dst_area->h > dst_buf->height)
            dst_h = dst_buf->height - dst_area->y;
        else
            dst_h = dst_area->h;
#ifdef DMA2D_ZOOM_WORKAROUND
        if (scale_y < 100 && (dst_area->y + dst_h) < dst_buf->height)
            dst_h++;
#endif

        dst_addr = (uint32_t)dst_buf->data + dst_area->y * dst_pitch
                    + dst_area->x * dma2d_get_bpp(dst_buf->format);
    } else {
        dst_w = dst_buf->width;
        dst_h = dst_buf->height;
        dst_addr = (uint32_t)dst_buf->data;
    }

    /* check zoom size and target area size */
    zoom_w = src_w * scale_x / 100;
    zoom_h = src_h * scale_y / 100;
    if (zoom_w > dst_w) {
        src_w = dst_w * 100 / scale_x;
    } else {
        dst_w = zoom_w;
    }
    if (zoom_h > dst_h) {
        src_h = dst_h * 100 / scale_y;
    } else {
        dst_h = zoom_h;
    }

    /* check target area width alignment */
    if (dst_buf->format == DMA2D_RGB565 && (dst_w % 4) != 0) {
        dst_w &= ~0x3;
        DMA2D_INFO("blend: target area width align to 4 pixels, dst_w = %d", dst_w);
    } else if ((dst_w % 2) != 0) {
        dst_w &= ~0x1;
        DMA2D_INFO("blend: target area width align to 2 pixels, dst_w = %d", dst_w);
    }

    /* no need to do blit/blend */
    if (src_w <= 0 || dst_w <= 0 || src_h <= 0 || dst_h <= 0) {
        DMA2D_INFO("blend: source or target area is zero, do nothing.");
        return DMA2D_OK;
    }

#ifdef DMA2D_ZOOM_WORKAROUND
    if (scale_x < 100 && (src_w + (src_area ? src_area->x : 0)) < src_buf->width)
        src_w++;
    if (scale_y < 100 && (src_h + (src_area ? src_area->y : 0)) < src_buf->height)
        src_h++;
#endif

    /* force memory address aligned */
    src_addr &= ~(DMA2D_ALIGN_BYTES - 1);
    dst_addr &= ~(DMA2D_ALIGN_BYTES - 1);

    /* reset dma2d for next work */
    dma2d_reset();

    /* framedone irq enable */
    dma2d_framedone_irq_enable();

    /* config layers */
    dma2d_config_gra_layer(src_w, src_h, dst_w, dst_h, src_buf->format, src_pitch, src_addr);
    if (blend != DMA2D_BLEND_NONE)
        dma2d_config_dma_layer(dst_w, dst_h, dst_w, dst_h, dst_buf->format, dst_pitch, dst_addr);

    /* config smart panel */
    dma2d_config_smart_panel(dst_w, dst_h, dst_buf->format, 0xff000000);

    /* config writeback layer */
#ifdef DMA2D_ZOOM_WORKAROUND
    if (scale_y < 100 && dst_h > 1)
        dst_h--;
#endif
    dma2d_config_out_layer(dst_w, dst_h, dst_buf->format, dst_pitch, dst_addr);

    dma2d_config_blend(blend, alpha, alpha);
    dma2d_config_process_path();
    dma2d_config_vsync(DMA2D_STBY_LEN);
    dma2d_config_burst_size();
    dma2d_frame_start();

    dma2d_working = true;
    DMA2D_INFO("blend: start %s buffer 0x%x -> 0x%x, mode %d, area %dx%d.",
                (blend == DMA2D_BLEND_NONE) ? "blit" : "blend",
                src_addr, dst_addr, blend, dst_w, dst_h);
    return DMA2D_OK;
}

int hal_dma2d_blit(struct DMA2D_BUFFER_T *src_buf, struct DMA2D_AREA_T *src_area,
                   struct DMA2D_BUFFER_T *dst_buf, struct DMA2D_AREA_T *dst_area,
                   int scale_x, int scale_y)
{
    return hal_dma2d_blend(src_buf, src_area, dst_buf, dst_area,
                           DMA2D_BLEND_NONE, 0, scale_x, scale_y);
}

int hal_dma2d_blend2(struct DMA2D_BUFFER_T *src_buf1, struct DMA2D_BUFFER_T *src_buf2,
                    struct DMA2D_BUFFER_T *dst_buf, struct DMA2D_AREA_T *dst_area,
                    enum DMA2D_BLEND_MODE_T blend, uint8_t alpha1, uint8_t alpha2)
{
    int dst_pitch = 0;
    int dst_w = 0;
    int dst_h = 0;
    uint32_t dst_addr;

    if ((!src_buf1 && !src_buf2) || !dst_buf) {
        DMA2D_ERR("blend2: source or target dma2d buffer is NULL.");
        return DMA2D_FAILED;
    }
    if ((src_buf1 && !src_buf1->data) ||
        (src_buf2 && !src_buf2->data) ||
        (!dst_buf->data)) {
        DMA2D_ERR("blend2: source or target buffer data address is NULL.");
        return DMA2D_FAILED;
    }
    if ((src_buf1 && (src_buf1->width == 0 || src_buf1->height == 0)) ||
        (src_buf2 && (src_buf2->width == 0 || src_buf2->height == 0)) ||
        (dst_buf->width == 0 || dst_buf->height == 0)) {
        DMA2D_ERR("blend2: source or target buffer size cannot be zero.");
        return DMA2D_FAILED;
    }
    if ((src_buf1 && src_buf1->width % DMA2D_ALIGN_PIXELS) ||
        (src_buf2 && src_buf2->width % DMA2D_ALIGN_PIXELS) ||
        (dst_buf->width % DMA2D_ALIGN_PIXELS)) {
        DMA2D_ERR("blend2: buffer width need align to %d pixels.", DMA2D_ALIGN_PIXELS);
        return DMA2D_UNALIGNMENT;
    }
    if ((src_buf1 && (((uint32_t)src_buf1->data) & (DMA2D_ALIGN_BYTES - 1))) ||
        (src_buf2 && (((uint32_t)src_buf2->data) & (DMA2D_ALIGN_BYTES - 1))) ||
        ((uint32_t)dst_buf->data) & (DMA2D_ALIGN_BYTES - 1)) {
        DMA2D_ERR("blend2: buffer address need align to %d bytes.", DMA2D_ALIGN_BYTES);
        return DMA2D_UNALIGNMENT;
    }

    dst_pitch = dst_buf->width * dma2d_get_bpp(dst_buf->format);

    if (dst_area) {
        if (dst_area->x < 0 || dst_area->x >= dst_buf->width ||
            dst_area->y < 0 || dst_area->y >= dst_buf->height) {
            DMA2D_ERR("blend2: invalid target area.");
            return DMA2D_FAILED;
        }

        if (dst_area->x + dst_area->w > dst_buf->width)
            dst_w = dst_buf->width - dst_area->x;
        else
            dst_w = dst_area->w;

        if (dst_area->y + dst_area->h > dst_buf->height)
            dst_h = dst_buf->height - dst_area->y;
        else
            dst_h = dst_area->h;

        dst_addr = (uint32_t)dst_buf->data + dst_area->y * dst_pitch
                    + dst_area->x * dma2d_get_bpp(dst_buf->format);
    } else {
        dst_w = dst_buf->width;
        dst_h = dst_buf->height;
        dst_addr = (uint32_t)dst_buf->data;
    }

    /* check target area width alignment */
    if (dst_buf->format == DMA2D_RGB565 && (dst_w % 4) != 0) {
        dst_w &= ~0x3;
        DMA2D_INFO("blend2: target area width align to 4 pixels, dst_w = %d", dst_w);
    } else if ((dst_w % 2) != 0) {
        dst_w &= ~0x1;
        DMA2D_INFO("blend2: target area width align to 2 pixels, dst_w = %d", dst_w);
    }

    /* no need to do blend */
    if (dst_w <= 0 || dst_h <= 0) {
        DMA2D_INFO("blend2: target area is zero, do nothing.");
        return DMA2D_OK;
    }

    /* force memory address aligned */
    dst_addr &= ~(DMA2D_ALIGN_BYTES - 1);

    /* reset dma2d for next work */
    dma2d_reset();

    /* framedone irq enable */
    dma2d_framedone_irq_enable();

    /* config layers */
    if (src_buf1) {
        dma2d_config_gra_layer(src_buf1->width, src_buf1->height, src_buf1->width, src_buf1->height,
                    src_buf1->format, src_buf1->width * dma2d_get_bpp(src_buf1->format),
                    (uint32_t)src_buf1->data);
    }
    if (src_buf2) {
        dma2d_config_dma_layer(src_buf2->width, src_buf2->height, src_buf2->width, src_buf2->height,
                    src_buf2->format, src_buf2->width * dma2d_get_bpp(src_buf2->format),
                    (uint32_t)src_buf2->data);
    }
    if (blend != DMA2D_BLEND_NONE)
        dma2d_config_tvg_layer(dst_w, dst_h, dst_w, dst_h, dst_buf->format, dst_pitch, dst_addr);

    /* config smart panel */
    dma2d_config_smart_panel(dst_w, dst_h, dst_buf->format, 0);

    /* config writeback layer */
    dma2d_config_out_layer(dst_w, dst_h, dst_buf->format, dst_pitch, dst_addr);

    dma2d_config_blend(blend, alpha1, alpha2);
    dma2d_config_process_path();
    dma2d_config_vsync(DMA2D_STBY_LEN);
    dma2d_config_burst_size();
    dma2d_frame_start();

    dma2d_working = true;
    DMA2D_INFO("blend2: start %s buffer 0x%x and 0x%x -> 0x%x, mode %d, area %dx%d.",
                (blend == DMA2D_BLEND_NONE) ? "blit" : "blend",
                src_buf1 ? (uint32_t)src_buf1->data : 0,
                src_buf2 ? (uint32_t)src_buf2->data : 0,
                dst_addr, blend, dst_w, dst_h);
    return DMA2D_OK;
}

bool hal_dma2d_busy(void)
{
    return dma2d_working;
}

void hal_dma2d_sleep(void)
{
    hal_cmu_dma2d_sleep();
}

void hal_dma2d_wakeup(void)
{
    hal_cmu_dma2d_wakeup();
}

#endif /* IMGDMA_BASE */
