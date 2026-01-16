/**
 * @file lv_draw_vglite_blend.c
 *
 */

/**
 * Copyright 2020-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_vglite.h"

#if LV_USE_DRAW_BES_VGLITE
#include <string.h>
#include "lv_vglite_buf.h"
#include "lv_vglite_matrix.h"
#include "lv_vglite_utils.h"
#include "lv_vglite_path.h"
#include "src/draw/lv_draw_image_private.h"
#include "src/draw/lv_image_decoder_private.h"

#include "hal_cache.h"

/*********************
 *      DEFINES
 *********************/
#define VGLITE_SWAP_INDEX1_BITS

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**
 * VGlite blit - fill a path with an image pattern
 *
 *
 * @param[in] dest_area Destination area with relative coordinates to dest buffer
 * @param[in] clip_area Clip area with relative coordinates to dest buff
 * @param[in] coords Coordinates of the image (relative to dest buff)
 * @param[in] dsc Image descriptor
 *
 */
static void _vglite_draw_pattern(const lv_area_t * src_area, const lv_area_t * clip_area,
                                 const lv_area_t * coords, const lv_draw_image_dsc_t * dsc);

/**
 * BLock Image Transfer - copy rectangular image from src_buf to dst_buf with or without effects.
 *
 * @param[in] src_area Source area with relative coordinates to src buffer
 * @param[in] dsc Image descriptor
 *
 */
static void _vglite_blit(const lv_area_t * src_area, const lv_draw_image_dsc_t * dsc);

static vg_lite_color_t _vglite_recolor(const lv_draw_image_dsc_t * dsc);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_vglite_img(lv_draw_task_t * t, const lv_draw_image_dsc_t * dsc,
                        const lv_area_t * coords)
{
    if(dsc->opa <= (lv_opa_t)LV_OPA_MIN)
        return;

    lv_layer_t * layer = t->target_layer;
    const void *src = (const void *)dsc->src;
    lv_image_src_t src_type = lv_image_src_get_type((const void *)dsc->src);
    const lv_image_dsc_t *img_dsc;
    lv_image_decoder_dsc_t decoder_dsc;
    bool need_decode = false;
    bool swap_rb = false;
    bool etc2_fmt = false;
    bool reset_clip_area = false;

    LV_PROFILER_DRAW_BEGIN;

    if (src_type == LV_IMAGE_SRC_VARIABLE) {
        img_dsc = src;
        if (img_dsc->header.cf == LV_COLOR_FORMAT_RAW)
            need_decode = true;
    } else {
        need_decode = true;
    }

    /* do image decode */
    if (need_decode) {
        if (lv_image_decoder_open(&decoder_dsc, src, NULL) != LV_RESULT_OK) {
            LV_LOG_ERROR("Failed to open image");
            return;
        }
        if (decoder_dsc.decoded == NULL) {
            LV_LOG_ERROR("Failed to decode image");
            return;
        }
        img_dsc = (lv_image_dsc_t *)decoder_dsc.decoded;
        if (img_dsc->data == NULL) {
            LV_LOG_ERROR("Failed to get image data");
            return;
        }
        /* if not a BES decoder, do cache sync */
        if (!(decoder_dsc.decoder->name && memcmp(decoder_dsc.decoder->name, "BES", 3) == 0)) {
            hal_cache_sync(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)img_dsc->data),
                    LEN_ALIGN_TO_CACHE((uint32_t)img_dsc->data_size, (uint32_t)img_dsc->data));
        }
        if (decoder_dsc.decoder->name && lv_strcmp(decoder_dsc.decoder->name, "BESPNG") == 0) {
            swap_rb = true;
        }
        if (decoder_dsc.decoder->name && lv_strcmp(decoder_dsc.decoder->name, "BESSVG") == 0) {
            swap_rb = true;
            reset_clip_area = true;
        }
        if (decoder_dsc.decoder->name && lv_strcmp(decoder_dsc.decoder->name, "BESJPEG") == 0) {
            reset_clip_area = true;
        }
        if (decoder_dsc.decoder->name && lv_strcmp(decoder_dsc.decoder->name, "BESETC2") == 0) {
            etc2_fmt = true;
        }
    }

    lv_area_t relative_coords;
    lv_area_copy(&relative_coords, coords);
    lv_area_move(&relative_coords, -layer->buf_area.x1, -layer->buf_area.y1);

    lv_area_t clip_area;
    lv_area_copy(&clip_area, &t->clip_area);
    lv_area_move(&clip_area, -layer->buf_area.x1, -layer->buf_area.y1);

    if (reset_clip_area) {
        vglite_set_scissor(&clip_area);
    }

    lv_area_t blend_area;
    bool has_transform = (dsc->rotation != 0 || dsc->scale_x != LV_SCALE_NONE || dsc->scale_y != LV_SCALE_NONE);
    if(has_transform)
        lv_area_copy(&blend_area, &relative_coords);
    else if(!lv_area_intersect(&blend_area, &relative_coords, &clip_area))
        goto exit; /*Fully clipped, nothing to do*/

    const void * src_buf = img_dsc->data;
    lv_color_format_t src_cf = img_dsc->header.cf;
    uint32_t src_w = img_dsc->header.w;
    uint32_t src_h = img_dsc->header.h;
    uint32_t src_stride = img_dsc->header.stride;
    uint8_t *tmpbuf = NULL;

    if (src_buf == NULL)
        goto exit;

    LV_LOG_TRACE("draw image %p, %dx%d, cf %d", src_buf, src_w, src_h, src_cf);

    lv_area_t src_area;
    src_area.x1 = blend_area.x1 - (coords->x1 - layer->buf_area.x1);
    src_area.y1 = blend_area.y1 - (coords->y1 - layer->buf_area.y1);
    src_area.x2 = src_w - 1;
    src_area.y2 = src_h - 1;

    /* config index-1/2/4/8 format */
    if (src_cf == LV_COLOR_FORMAT_I1 || src_cf == LV_COLOR_FORMAT_I2 ||
        src_cf == LV_COLOR_FORMAT_I4 || src_cf == LV_COLOR_FORMAT_I8) {
        uint32_t *colors = (uint32_t *)img_dsc->data;

        vg_lite_set_CLUT(2, colors);
        vg_lite_set_CLUT(4, colors);
        vg_lite_set_CLUT(16, colors);
        vg_lite_set_CLUT(256, colors);

        if (src_cf == LV_COLOR_FORMAT_I1)
            src_buf += 8;
        else if (src_cf == LV_COLOR_FORMAT_I2)
            src_buf += 16;
        else if (src_cf == LV_COLOR_FORMAT_I4)
            src_buf += 64;
        else
            src_buf += 1024;
    }

    /* Set src_vgbuf structure. */
#ifdef VGLITE_SWAP_INDEX1_BITS
    if (!vglite_buf_aligned(src_buf, src_w, src_stride, src_cf) || src_cf == LV_COLOR_FORMAT_I1) {
#else
    if (!vglite_buf_aligned(src_buf, src_w, src_stride, src_cf)) {
#endif
        uint32_t aligned_w = (src_w + 15) / 16 * 16;
        uint32_t aligned_stride = vglite_get_buf_stride(src_cf, aligned_w);
        uint32_t cp_stride = (aligned_stride > src_stride) ? src_stride : aligned_stride;

        tmpbuf = lv_malloc_zeroed(aligned_stride * src_h + 64);
        if (tmpbuf == NULL)
            goto exit;

        uint8_t *aligned_buf = (uint8_t *)(((uint32_t)tmpbuf + 63) & 0xffffffc0);
        const uint8_t *p = src_buf;
        uint8_t *q = aligned_buf;
        for (int row = 0; row < src_h; row++) {
#ifdef VGLITE_SWAP_INDEX1_BITS
            if (src_cf == LV_COLOR_FORMAT_I1) {
                for (int col = 0; col < src_w / 8; col++) {
                    uint8_t d = p[col];
                    if (d > 0 && d < 0xff) {
                        q[col] = ((d & 0x80) >> 7) | ((d & 0x40) >> 5) | ((d & 0x20) >> 3) |
                                 ((d & 0x10) >> 1) | ((d & 0x8) << 1) | ((d & 0x4) << 3) |
                                 ((d & 0x2) << 5) | ((d & 0x1) << 7);
                    } else {
                        q[col] = d;
                    }
                }
                p += src_stride;
                q += aligned_stride;
                continue;
            }
#endif
            lv_memcpy(q, p, cp_stride);
            p += src_stride;
            q += aligned_stride;
        }
        hal_cache_sync(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)aligned_buf),
                       LEN_ALIGN_TO_CACHE(aligned_stride * src_h, (uint32_t)aligned_buf));
        vglite_set_src_buf(aligned_buf, src_w, src_h, aligned_stride, src_cf);
    } else {
        vglite_set_src_buf(src_buf, src_w, src_h, src_stride, src_cf);
    }

    if (swap_rb) {
        vglite_set_srcbuf_swaprb();
    }

    if (etc2_fmt) {
        vglite_set_srcbuf_etc2_format();
    }

    vglite_set_transformation_matrix(&blend_area, dsc);
    bool is_tiled = dsc->tile;
    if(is_tiled)
        _vglite_draw_pattern(&src_area, &clip_area, &relative_coords, dsc);
    else
        _vglite_blit(&src_area, dsc);

    if (tmpbuf)
        lv_free(tmpbuf);

exit:
    if (need_decode) {
        lv_image_decoder_close(&decoder_dsc);
    }

    LV_PROFILER_DRAW_END;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void _vglite_blit(const lv_area_t * src_area, const lv_draw_image_dsc_t * dsc)
{
    vg_lite_buffer_t * dst_vgbuf = vglite_get_dest_buf();
    vg_lite_buffer_t * src_vgbuf = vglite_get_src_buf();

    vg_lite_rectangle_t rect = {
        .x = (vg_lite_int32_t)src_area->x1,
        .y = (vg_lite_int32_t)src_area->y1,
        .width = (vg_lite_int32_t)lv_area_get_width(src_area),
        .height = (vg_lite_int32_t)lv_area_get_height(src_area)
    };

    src_vgbuf->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    src_vgbuf->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;

    vg_lite_color_t vgcol = 0xffffffff;
    bool has_recolor = (dsc->recolor_opa > LV_OPA_MIN);
    if (has_recolor) {
        vgcol = _vglite_recolor(dsc);
        src_vgbuf->image_mode = VG_LITE_RECOLOR_MODE;
    }

    vg_lite_matrix_t * vgmatrix = vglite_get_matrix();
    vg_lite_blend_t vgblend = vglite_get_blend_mode(dsc->blend_mode);

    VGLITE_CHECK_ERROR(vg_lite_blit_rect(dst_vgbuf, src_vgbuf, &rect, vgmatrix, vgblend, vgcol, VG_LITE_FILTER_POINT));

    vglite_run();
}

static void _vglite_draw_pattern(const lv_area_t * src_area, const lv_area_t * clip_area,
                                 const lv_area_t * coords, const lv_draw_image_dsc_t * dsc)
{
#if gcFEATURE_VG_IM_REPEAT_REFLECT
    /* Target buffer */
    vg_lite_buffer_t * dst_vgbuf = vglite_get_dest_buf();

    /* Path to draw */
    int32_t path_data[RECT_PATH_DATA_MAX_SIZE];
    uint32_t path_data_size;
    vglite_create_rect_path_data(path_data, &path_data_size, 0, coords);
    vg_lite_quality_t path_quality = VG_LITE_MEDIUM;

    vg_lite_path_t path;
    VGLITE_CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_S32, path_quality, path_data_size, path_data,
                                         (vg_lite_float_t)clip_area->x1, (vg_lite_float_t)clip_area->y1,
                                         ((vg_lite_float_t)clip_area->x2) + 1.0f, ((vg_lite_float_t)clip_area->y2) + 1.0f));

    /* Path Matrix */
    vg_lite_matrix_t path_matrix;
    vg_lite_identity(&path_matrix);

    /* Pattern Image */
    vg_lite_buffer_t * src_vgbuf = vglite_get_src_buf();
    src_vgbuf->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    src_vgbuf->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;

    /* Pattern matrix */
    vg_lite_matrix_t * vgmatrix = vglite_get_matrix();

    /* Blend mode */
    vg_lite_blend_t vgblend = vglite_get_blend_mode(dsc->blend_mode);

    vg_lite_color_t vgcol = _vglite_recolor(dsc);

    /* Filter */
    bool has_trasform = (dsc->rotation != 0 || dsc->scale_x != LV_SCALE_NONE || dsc->scale_y != LV_SCALE_NONE);
    vg_lite_filter_t filter = has_trasform ? VG_LITE_FILTER_BI_LINEAR : VG_LITE_FILTER_POINT;

    /* Draw Pattern */
    VGLITE_CHECK_ERROR(vg_lite_draw_pattern(dst_vgbuf, &path, VG_LITE_FILL_NON_ZERO, &path_matrix,
                                            src_vgbuf, vgmatrix, vgblend, VG_LITE_PATTERN_REPEAT,
                                            0, vgcol, filter));
#else
    vg_lite_buffer_t * dst_vgbuf = vglite_get_dest_buf();
    vg_lite_buffer_t * src_vgbuf = vglite_get_src_buf();

    lv_area_t tile_area;
    if(lv_area_get_width(&dsc->image_area) >= 0)
        tile_area = dsc->image_area;
    else
        tile_area = *coords;

    vg_lite_rectangle_t rect = {
        .x = (vg_lite_int32_t)src_area->x1,
        .y = (vg_lite_int32_t)src_area->y1,
        .width = (vg_lite_int32_t)lv_area_get_width(src_area),
        .height = (vg_lite_int32_t)lv_area_get_height(src_area)
    };

    if (src_vgbuf->format == VG_LITE_A8 || src_vgbuf->format == VG_LITE_A4) {
        src_vgbuf->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
        src_vgbuf->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    }

    vg_lite_color_t vgcol = _vglite_recolor(dsc);

    vg_lite_matrix_t matrix;
    vg_lite_blend_t vgblend = vglite_get_blend_mode(dsc->blend_mode);

    for (int y = tile_area.y1; y < clip_area->y2;) {
        for (int x = tile_area.x1; x < clip_area->x2;) {
            vg_lite_identity(&matrix);
            vg_lite_rotate(dsc->rotation / 10.0f, &matrix);
            vg_lite_scale(dsc->scale_x / 256.0f, dsc->scale_y / 256.0f, &matrix);
            vg_lite_translate(x, y, &matrix);
            VGLITE_CHECK_ERROR(vg_lite_blit_rect(dst_vgbuf, src_vgbuf, &rect, &matrix, vgblend, vgcol, VG_LITE_FILTER_POINT));

            if (x == clip_area->x1) {
                x += rect.width * dsc->scale_x / 256;
                rect.x = 0;
                rect.width = dsc->header.w;
            } else {
                x += rect.width * dsc->scale_x / 256;
            }
        }
        if (y == clip_area->y1) {
            y += rect.height * dsc->scale_y / 256;
            rect.y = 0;
            rect.height = dsc->header.h;
        } else {
            y += rect.height * dsc->scale_y / 256;
        }
    }
    vglite_run();
#endif
}

static vg_lite_color_t _vglite_recolor(const lv_draw_image_dsc_t * dsc)
{
    lv_color_t color;
    lv_opa_t opa;

    bool has_recolor = (dsc->recolor_opa > LV_OPA_MIN);
    if(has_recolor) {
        color = dsc->recolor;
        opa = LV_OPA_MIX2(dsc->recolor_opa, dsc->opa);
    }
    else {
        color.red = 0xFF;
        color.green = 0xFF;
        color.blue = 0xFF;
        opa = dsc->opa;
    }

    lv_color32_t col32 = lv_color_to_32(color, opa);

    return vglite_get_color(col32, false);
}

#endif /*LV_USE_DRAW_BES_VGLITE*/
