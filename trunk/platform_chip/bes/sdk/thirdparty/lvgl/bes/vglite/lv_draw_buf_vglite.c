/**
 * @file lv_draw_buf_vglite.c
 *
 */

/**
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_vglite.h"

#if LV_USE_DRAW_BES_VGLITE
#include "src/draw/lv_draw_buf_private.h"
#include "lv_vglite_buf.h"
#include "lv_vglite_utils.h"

#include "hal_cache.h"

/*********************
 *      DEFINES
 *********************/
#define VGLITE_BUF_ALIGNMENT        64

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void * _buf_malloc(size_t size_bytes, lv_color_format_t cf);

static void * _buf_align(void * buf, lv_color_format_t cf);

static void _invalidate_cache(const lv_draw_buf_t * draw_buf, const lv_area_t * area);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_buf_vglite_init_handlers(void)
{
    lv_draw_buf_handlers_t * handlers = lv_draw_buf_get_handlers();
    lv_draw_buf_handlers_t * font_handlers = lv_draw_buf_get_font_handlers();
    lv_draw_buf_handlers_t * image_handlers = lv_draw_buf_get_image_handlers();

    handlers->buf_malloc_cb = _buf_malloc;
    handlers->align_pointer_cb = _buf_align;
    handlers->invalidate_cache_cb = _invalidate_cache;

    font_handlers->buf_malloc_cb = _buf_malloc;
    font_handlers->align_pointer_cb = _buf_align;
    font_handlers->invalidate_cache_cb = _invalidate_cache;

    image_handlers->buf_malloc_cb = _buf_malloc;
    image_handlers->align_pointer_cb = _buf_align;
    image_handlers->invalidate_cache_cb = _invalidate_cache;
}

void lv_draw_vglite_sync_invalidate(const void *data, uint32_t size)
{
    uint32_t addr = (uint32_t)data;

    hal_cache_sync_invalidate(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE(addr),
                              LEN_ALIGN_TO_CACHE(size, addr));
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void * _buf_malloc(size_t size_bytes, lv_color_format_t cf)
{
    uint8_t align_bytes = VGLITE_BUF_ALIGNMENT;

    /*Allocate larger memory to be sure it can be aligned as needed*/
    size_bytes += align_bytes - 1;

    return lv_malloc(size_bytes);
}

static void * _buf_align(void * buf, lv_color_format_t cf)
{
    uint8_t align_bytes = VGLITE_BUF_ALIGNMENT;

    uint8_t * buf_u8 = buf;
    if(buf_u8) {
        buf_u8 += align_bytes - 1;
        buf_u8 = (uint8_t *)((lv_uintptr_t)buf_u8 & ~(align_bytes - 1));
    }

    return buf_u8;
}

static void _invalidate_cache(const lv_draw_buf_t * draw_buf, const lv_area_t * area)
{
    lv_draw_vglite_sync_invalidate(draw_buf->data, draw_buf->data_size);
}

#endif /*LV_USE_DRAW_BES_VGLITE*/
