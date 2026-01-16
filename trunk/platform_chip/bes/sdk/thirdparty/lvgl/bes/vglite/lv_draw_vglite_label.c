/**
 * @file lv_draw_vglite_label.c
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
#include "lv_vglite_buf.h"
#include "lv_vglite_matrix.h"
#include "lv_vglite_utils.h"
#include "src/draw/lv_draw_label_private.h"
#include "lv_vglite_freetype.h"

#include "hal_location.h"
#include "hal_cache.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void _draw_vglite_letter(lv_draw_task_t * t, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area);

/**
 * Draw letter (character bitmap blend) with optional color and opacity
 *
 * @param[in] mask_area Mask area with relative coordinates of source buffer
 * @param[in] color Color
 * @param[in] opa Opacity
 *
 */
static void _vglite_draw_letter(const lv_area_t * mask_area, lv_color_t color, lv_opa_t opa);

#if LV_USE_FREETYPE
/**
 * Draw freetype outline letter.
 */
static void _vglite_draw_letter_outline(lv_draw_task_t * t, lv_draw_glyph_dsc_t * glyph_draw_dsc);
#endif

static void _vglite_letter_buf_sync(void);

/**********************
 *  STATIC VARIABLES
 **********************/
#define LETTER_BUF_WIDTH   32
#define LETTER_BUF_HEIGHT  32
#define LETTER_BUF_CNT     32

#ifdef PSRAM_ENABLE
PSRAM_NC_BSS_LOC
#else
LV_ATTRIBUTE_LARGE_RAM_ARRAY
#endif
static ALIGNED(64) uint8_t letter_buf[LETTER_BUF_CNT][LETTER_BUF_WIDTH * LETTER_BUF_HEIGHT];
static uint32_t letter_cnt = 0;

/**********************
 *  GLOBAL VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_vglite_label(lv_draw_task_t * t, const lv_draw_label_dsc_t * dsc,
                          const lv_area_t * coords)
{
    if (dsc->opa <= LV_OPA_MIN) return;

    LV_PROFILER_DRAW_BEGIN;

#if LV_USE_FREETYPE
    vglite_freetype_init_outline_cb();
#endif

    letter_cnt = 0;

    lv_draw_label_iterate_characters(t, dsc, coords, _draw_vglite_letter);

    if (letter_cnt > 0) {
        _vglite_letter_buf_sync();
        vglite_run();
    }

    LV_PROFILER_DRAW_END;
}

void lv_draw_vglite_letter(lv_draw_task_t * t, const lv_draw_letter_dsc_t * dsc,
                           const lv_area_t * coords)
{
    if (dsc->opa <= LV_OPA_MIN)
        return;

    LV_PROFILER_DRAW_BEGIN;

    lv_draw_glyph_dsc_t glyph_dsc;
    lv_draw_glyph_dsc_init(&glyph_dsc);
    glyph_dsc.opa = dsc->opa;
    glyph_dsc.bg_coords = NULL;
    glyph_dsc.color = dsc->color;
    glyph_dsc.rotation = dsc->rotation;
    glyph_dsc.pivot = dsc->pivot;

    lv_point_t pos = { .x = coords->x1, .y = coords->y1 };

    lv_draw_unit_draw_letter(t, &glyph_dsc, &pos, dsc->font, dsc->unicode, _draw_vglite_letter);

    if (glyph_dsc._draw_buf) {
        lv_draw_buf_destroy(glyph_dsc._draw_buf);
        glyph_dsc._draw_buf = NULL;
    }

    LV_PROFILER_DRAW_END;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void _draw_vglite_letter(lv_draw_task_t * t, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area)
{
    if(glyph_draw_dsc) {
        switch(glyph_draw_dsc->format) {

            case LV_FONT_GLYPH_FORMAT_NONE: {
#if LV_USE_FONT_PLACEHOLDER
                    /* Draw a placeholder rectangle*/
                    lv_draw_border_dsc_t border_draw_dsc;
                    lv_draw_border_dsc_init(&border_draw_dsc);
                    border_draw_dsc.opa = glyph_draw_dsc->opa;
                    border_draw_dsc.color = glyph_draw_dsc->color;
                    border_draw_dsc.width = 1;
                    lv_draw_vglite_border(t, &border_draw_dsc, glyph_draw_dsc->bg_coords);
#endif
                }
                break;
            case LV_FONT_GLYPH_FORMAT_A1:
            case LV_FONT_GLYPH_FORMAT_A2:
            case LV_FONT_GLYPH_FORMAT_A4:
            case LV_FONT_GLYPH_FORMAT_A8:
            {
                    /*Do not draw transparent things*/
                    if(glyph_draw_dsc->opa <= LV_OPA_MIN)
                        return;

                    lv_layer_t * layer = t->target_layer;

                    lv_area_t blend_area;
                    if(!lv_area_intersect(&blend_area, glyph_draw_dsc->letter_coords, &t->clip_area))
                        return;
                    lv_area_move(&blend_area, -layer->buf_area.x1, -layer->buf_area.y1);

                    lv_color_format_t mask_format = LV_COLOR_FORMAT_A8;;
                    uint32_t mask_width, mask_height, mask_stride;
                    const void *mask_buf;

                    if (lv_font_has_static_bitmap(glyph_draw_dsc->g->resolved_font)) {
                        mask_buf = lv_font_get_glyph_static_bitmap(glyph_draw_dsc->g);
                        mask_width = glyph_draw_dsc->g->box_w;
                        mask_height = glyph_draw_dsc->g->box_h;
                        mask_stride = glyph_draw_dsc->g->stride;
                    } else {
                        const lv_draw_buf_t *draw_buf = lv_font_get_glyph_bitmap(glyph_draw_dsc->g, glyph_draw_dsc->_draw_buf);
                        if (!draw_buf)
                            return;

                        mask_buf = draw_buf->data;
                        mask_width = draw_buf->header.w;
                        mask_height = draw_buf->header.h;
                        mask_stride = draw_buf->header.stride;
                        mask_format = draw_buf->header.cf;
                    }

                    if (!mask_buf)
                        return;

                    lv_area_t mask_area;
                    mask_area.x1 = blend_area.x1 - (glyph_draw_dsc->letter_coords->x1 - layer->buf_area.x1);
                    mask_area.y1 = blend_area.y1 - (glyph_draw_dsc->letter_coords->y1 - layer->buf_area.y1);
                    mask_area.x2 = mask_width - 1;
                    mask_area.y2 = mask_height - 1;

                    if (!vglite_buf_aligned(mask_buf, mask_width, mask_stride, mask_format)) {
#if 0
                        /* Draw a placeholder rectangle*/
                        lv_draw_border_dsc_t border_draw_dsc;
                        lv_draw_border_dsc_init(&border_draw_dsc);
                        border_draw_dsc.opa = glyph_draw_dsc->opa;
                        border_draw_dsc.color = glyph_draw_dsc->color;
                        border_draw_dsc.width = 1;
                        lv_draw_vglite_border(t, &border_draw_dsc, glyph_draw_dsc->bg_coords);
#else
                        /* aligned to 16 pixels */
                        uint32_t aligned_width = (mask_width + 15) / 16 * 16;
                        uint32_t aligned_stride = vglite_get_buf_stride(mask_format, aligned_width);
                        if (aligned_width * mask_height > LETTER_BUF_WIDTH * LETTER_BUF_HEIGHT) {
                            uint8_t *tmpbuf = lv_malloc_zeroed(aligned_width * mask_height + 64);
                            if (tmpbuf == NULL) {
                                /* Draw a placeholder rectangle*/
                                lv_draw_border_dsc_t border_draw_dsc;
                                lv_draw_border_dsc_init(&border_draw_dsc);
                                border_draw_dsc.opa = glyph_draw_dsc->opa;
                                border_draw_dsc.color = glyph_draw_dsc->color;
                                border_draw_dsc.width = 1;
                                lv_draw_vglite_border(t, &border_draw_dsc, glyph_draw_dsc->bg_coords);
                            } else {
                                uint8_t *aligned_buf = (uint8_t *)(((uint32_t)tmpbuf + 63) & 0xffffffc0);
                                uint8_t *q = aligned_buf;
                                const uint8_t *p = mask_buf;
                                for (int row = 0; row < mask_height; row++) {
                                    lv_memcpy(q, p, mask_stride);
                                    q += aligned_stride;
                                    p += mask_stride;
                                }
                                hal_cache_sync(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)aligned_buf),
                                            LEN_ALIGN_TO_CACHE(aligned_stride * mask_height, (uint32_t)aligned_buf));
                                vglite_set_src_buf(aligned_buf, aligned_width, mask_height, aligned_stride, mask_format);
                                vglite_set_translation_matrix(&blend_area);
                                _vglite_draw_letter(&mask_area, glyph_draw_dsc->color, glyph_draw_dsc->opa);
                                vglite_run();
                                lv_free(tmpbuf);
                            }
                        } else {
                            uint8_t *tmpbuf = letter_buf[letter_cnt];
                            uint8_t *q = tmpbuf;
                            const uint8_t *p = mask_buf;
                            letter_cnt++;

                            lv_memset(tmpbuf, 0, aligned_stride * mask_height);
                            for (int row = 0; row < mask_height; row++) {
                                lv_memcpy(q, p, mask_stride);
                                q += aligned_stride;
                                p += mask_stride;
                            }
                            vglite_set_src_buf(tmpbuf, aligned_width, mask_height, aligned_stride, mask_format);
                            vglite_set_translation_matrix(&blend_area);
                            _vglite_draw_letter(&mask_area, glyph_draw_dsc->color, glyph_draw_dsc->opa);
                            if (letter_cnt >= LETTER_BUF_CNT) {
                                _vglite_letter_buf_sync();
                                vglite_run();
                                letter_cnt = 0;
                            }
                        }
#endif
                    }
                    else {
                        if (mask_width * mask_height > LETTER_BUF_WIDTH * LETTER_BUF_HEIGHT) {
                            vglite_set_src_buf(mask_buf, mask_width, mask_height, mask_stride, mask_format);
                            vglite_set_translation_matrix(&blend_area);
                            lv_draw_vglite_sync_invalidate(mask_buf, mask_stride * mask_height);
                            _vglite_draw_letter(&mask_area, glyph_draw_dsc->color, glyph_draw_dsc->opa);
                            vglite_run();
                        } else {
                            uint8_t *tmpbuf = letter_buf[letter_cnt];
                            letter_cnt++;

                            lv_memcpy(tmpbuf, mask_buf, mask_stride * mask_height);
                            vglite_set_src_buf(tmpbuf, mask_width, mask_height, mask_stride, mask_format);
                            vglite_set_translation_matrix(&blend_area);
                            _vglite_draw_letter(&mask_area, glyph_draw_dsc->color, glyph_draw_dsc->opa);
                            if (letter_cnt >= LETTER_BUF_CNT) {
                                _vglite_letter_buf_sync();
                                vglite_run();
                                letter_cnt = 0;
                            }
                        }
                    }
                }
                break;
            case LV_FONT_GLYPH_FORMAT_IMAGE: {
#if LV_USE_IMGFONT
                    lv_draw_image_dsc_t img_dsc;
                    lv_draw_image_dsc_init(&img_dsc);
                    img_dsc.rotation = 0;
                    img_dsc.scale_x = LV_SCALE_NONE;
                    img_dsc.scale_y = LV_SCALE_NONE;
                    img_dsc.opa = glyph_draw_dsc->opa;
                    img_dsc.src = glyph_draw_dsc->glyph_data;
                    lv_draw_vglite_img(t, &img_dsc, glyph_draw_dsc->letter_coords);
#endif
                }
                break;
#if LV_USE_FREETYPE
            case LV_FONT_GLYPH_FORMAT_VECTOR: {
                    if (lv_freetype_is_outline_font(glyph_draw_dsc->g->resolved_font))
                        _vglite_draw_letter_outline(t, glyph_draw_dsc);
                }
                break;
#endif
            default:
                break;
        }
    }

    if(fill_draw_dsc && fill_area) {
        lv_draw_vglite_fill(t, fill_draw_dsc, fill_area);
    }
}

static void _vglite_draw_letter(const lv_area_t * mask_area, lv_color_t color, lv_opa_t opa)
{
    vg_lite_buffer_t * dst_vgbuf = vglite_get_dest_buf();
    vg_lite_buffer_t * mask_vgbuf = vglite_get_src_buf();

    mask_vgbuf->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    mask_vgbuf->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;

    vg_lite_rectangle_t rect = {
        .x = (vg_lite_int32_t)mask_area->x1,
        .y = (vg_lite_int32_t)mask_area->y1,
        .width = (vg_lite_int32_t)lv_area_get_width(mask_area),
        .height = (vg_lite_int32_t)lv_area_get_height(mask_area)
    };

    lv_color32_t col32 = lv_color_to_32(color, opa);
    vg_lite_color_t vgcol = vglite_get_color(col32, false);

    vg_lite_matrix_t * vgmatrix = vglite_get_matrix();

    /*Blit with font color as paint color*/
    VGLITE_CHECK_ERROR(vg_lite_blit_rect(dst_vgbuf, mask_vgbuf, &rect, vgmatrix, VG_LITE_BLEND_SRC_OVER, vgcol,
                                         VG_LITE_FILTER_POINT));
}

#if LV_USE_FREETYPE
static void _vglite_draw_letter_outline(lv_draw_task_t * t,
                                        lv_draw_glyph_dsc_t * glyph_draw_dsc)
{
    VGLITE_CHECK_ERROR(vglite_freetype_draw(t, glyph_draw_dsc));
    vglite_run();
}
#endif /* LV_USE_FREETYPE */

static void _vglite_letter_buf_sync(void)
{
    uint32_t addr, size;

    if (letter_cnt == 0)
        return;

    addr = (uint32_t)letter_buf;
    size = letter_cnt * LETTER_BUF_WIDTH * LETTER_BUF_HEIGHT;

#if defined(PSRAM_NC_BASE) && defined(PSRAM_SIZE)
    if (addr >= PSRAM_NC_BASE && addr < (PSRAM_NC_BASE + PSRAM_SIZE))
        return;
#endif
    hal_cache_sync(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE(addr),
                   LEN_ALIGN_TO_CACHE(size, addr));
}

#endif /*LV_USE_DRAW_BES_VGLITE*/
