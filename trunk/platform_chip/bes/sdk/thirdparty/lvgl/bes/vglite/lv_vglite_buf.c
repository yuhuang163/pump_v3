/**
 * @file lv_vglite_buf.c
 *
 */

/**
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: MIT
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_vglite.h"

#if LV_USE_DRAW_BES_VGLITE
#include "lv_vglite_buf.h"
#include "lv_vglite_utils.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static inline void _set_vgbuf_ptr(vg_lite_buffer_t * vgbuf, void * buf);
#if LV_USE_BES_VGLITE_COMPRESS
static void _set_vgbuf_compress(vg_lite_buffer_t * vgbuf, void * buf,
        uint32_t width, uint32_t height, uint32_t stride, lv_color_format_t cf,
        vg_lite_compress_mode_t mode);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

static vg_lite_buffer_t _dest_vgbuf;
static vg_lite_buffer_t _src_vgbuf;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#include "gpu_port.h"

uint32_t lv_vglite_gpu_addr(uint32_t addr)
{
    return gpu_hal_buf_addr(addr);
}

vg_lite_buffer_t * vglite_get_dest_buf(void)
{
    return &_dest_vgbuf;
}

vg_lite_buffer_t * vglite_get_src_buf(void)
{
    return &_src_vgbuf;
}

void vglite_set_dest_buf_ptr(void * buf)
{
    _set_vgbuf_ptr(&_dest_vgbuf, buf);
}

void vglite_set_src_buf_ptr(const void * buf)
{
    _set_vgbuf_ptr(&_src_vgbuf, (void *)buf);
}

void vglite_set_dest_buf(const void * buf, uint32_t width, uint32_t height, uint32_t stride,
                         lv_color_format_t cf)
{
#if LV_USE_BES_VGLITE_COMPRESS
    lv_draw_buf_t *draw_buf = lv_display_get_buf_active(lv_display_get_default());
    if (buf == draw_buf->data) { /* only enable compress for active framebuffer */
        _set_vgbuf_compress(&_dest_vgbuf, (void *)buf, width, height, stride, cf,
#if (LV_BES_VGLITE_COMPRESS_MODE == 2)
                    VG_LITE_DEC_HSAMPLE
#else
                    VG_LITE_DEC_NON_SAMPLE
#endif
        );
    } else {
        vglite_set_buf(&_dest_vgbuf, (void *)buf, width, height, stride, cf);
    }
#else
    vglite_set_buf(&_dest_vgbuf, (void *)buf, width, height, stride, cf);
#endif
}

void vglite_set_src_buf(const void * buf, uint32_t width, uint32_t height, uint32_t stride,
                        lv_color_format_t cf)
{
    vglite_set_buf(&_src_vgbuf, (void *)buf, width, height, stride, cf);
}

void vglite_set_buf(vg_lite_buffer_t * vgbuf, void * buf,
                    uint32_t width, uint32_t height, uint32_t stride,
                    lv_color_format_t cf)
{
    vg_lite_buffer_format_t vgformat = vglite_get_buf_format(cf);

    if (stride == 0)
        stride = vglite_get_buf_stride(cf, width);

    lv_memzero(vgbuf, sizeof(vg_lite_buffer_t));

    vgbuf->format = vgformat;
    vgbuf->tiled = VG_LITE_LINEAR;
    vgbuf->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    vgbuf->transparency_mode = VG_LITE_IMAGE_OPAQUE;

    vgbuf->width = (int32_t)width;
    vgbuf->height = (int32_t)height;
    vgbuf->stride = (int32_t)stride;

    lv_memzero(&vgbuf->yuv, sizeof(vgbuf->yuv));

    vgbuf->memory = buf;
    vgbuf->address = lv_vglite_gpu_addr((uint32_t)vgbuf->memory);
    vgbuf->handle = NULL;
}

void vglite_set_srcbuf_swaprb(void)
{
    vglite_set_buf_swaprb(&_src_vgbuf);
}

void vglite_set_destbuf_swaprb(void)
{
    vglite_set_buf_swaprb(&_dest_vgbuf);
}

void vglite_set_buf_swaprb(vg_lite_buffer_t *vgbuf)
{
    switch (vgbuf->format) {
        case VG_LITE_BGR565:
            vgbuf->format = VG_LITE_RGB565;
            break;
        case VG_LITE_RGB565:
            vgbuf->format = VG_LITE_BGR565;
            break;
        case VG_LITE_ABGR8565:
            vgbuf->format = VG_LITE_ARGB8565;
            break;
        case VG_LITE_ARGB8565:
            vgbuf->format = VG_LITE_ABGR8565;
            break;
        case VG_LITE_BGR888:
            vgbuf->format = VG_LITE_RGB888;
            break;
        case VG_LITE_RGB888:
            vgbuf->format = VG_LITE_BGR888;
            break;
        case VG_LITE_BGRA8888:
            vgbuf->format = VG_LITE_RGBA8888;
            break;
        case VG_LITE_RGBA8888:
            vgbuf->format = VG_LITE_BGRA8888;
            break;
        case VG_LITE_BGRX8888:
            vgbuf->format = VG_LITE_RGBX8888;
            break;
        case VG_LITE_RGBX8888:
            vgbuf->format = VG_LITE_BGRX8888;
            break;
        default:
            break;
    };
}

void vglite_set_srcbuf_etc2_format(void)
{
    vg_lite_buffer_t *vgbuf = &_src_vgbuf;
    vgbuf->format = VG_LITE_RGBA8888_ETC2_EAC;
    vgbuf->tiled = VG_LITE_TILED;
    vgbuf->stride = vgbuf->width;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline void _set_vgbuf_ptr(vg_lite_buffer_t * vgbuf, void * buf)
{
    vgbuf->memory = buf;
    vgbuf->address = lv_vglite_gpu_addr((uint32_t)vgbuf->memory);
}

#if LV_USE_BES_VGLITE_COMPRESS
static void _set_vgbuf_compress(vg_lite_buffer_t * vgbuf, void * buf,
        uint32_t width, uint32_t height, uint32_t stride, lv_color_format_t cf,
        vg_lite_compress_mode_t mode)
{
    vg_lite_buffer_format_t vgformat = vglite_get_buf_format(cf);

    lv_memzero(vgbuf, sizeof(vg_lite_buffer_t));

    vgbuf->format = vgformat;
    vgbuf->width = (int32_t)width;
    vgbuf->height = (int32_t)height;
    vgbuf->stride = (int32_t)stride;
    vgbuf->memory = buf;
    vgbuf->address = lv_vglite_gpu_addr((uint32_t)vgbuf->memory);

    if (mode == VG_LITE_DEC_HV_SAMPLE) {
        vgbuf->compress_mode = mode;
        vgbuf->tiled = VG_LITE_TILED;
    } else if (mode == VG_LITE_DEC_NON_SAMPLE || mode == VG_LITE_DEC_HSAMPLE) {
        vgbuf->compress_mode = mode;
        vgbuf->tiled = VG_LITE_LINEAR;
    } else {
        vgbuf->compress_mode = VG_LITE_DEC_DISABLE;
        vgbuf->tiled = VG_LITE_LINEAR;
    }
}
#endif

#endif /*LV_USE_DRAW_BES_VGLITE*/
