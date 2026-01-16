/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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

#ifndef __BESVG_H__
#define __BESVG_H__

#include <stdint.h>

typedef enum {
    BESVG_OK = 0,
    BESVG_FAILED,
} BESVG_RESULT_T;

/* The bits of each color channel are stored from LSB to MSB;
 * for example: RGBA8888, R - b0:7, G - b8:15, B - b16:23, A - 24:31 */
typedef enum {
    BESVG_RGBA8888                  = 0,
    BESVG_BGRA8888                  = 1,
    BESVG_RGBX8888                  = 2,
    BESVG_BGRX8888                  = 3,

    BESVG_RGB565                    = 4,
    BESVG_BGR565                    = 5,

    BESVG_RGBA4444                  = 6,
    BESVG_BGRA4444                  = 7,
    BESVG_ABGR4444                  = 29,
    BESVG_ARGB4444                  = 30,

    BESVG_RGBA2222                  = 25,
    BESVG_BGRA2222                  = 26,
    BESVG_ABGR2222                  = 27,
    BESVG_ARGB2222                  = 28,

    BESVG_ABGR8888                  = 31,
    BESVG_ARGB8888                  = 32,
    BESVG_XBGR8888                  = 36,
    BESVG_XRGB8888                  = 37,

    BESVG_RGB888                    = 39,
    BESVG_BGR888                    = 40,

} BESVG_FORMAT_T;

typedef enum {
    BESVG_FILL_RULE_EVEN_ODD        = 0,
    BESVG_FILL_RULE_NON_ZERO        = 1,
} BESVG_FILL_RULE_T;

typedef enum {
    BESVG_LINE_CAP_BUTT             = 0,
    BESVG_LINE_CAP_ROUND            = 1,
    BESVG_LINE_CAP_SQUARE           = 2,
} BESVG_LINE_CAP_T;

typedef enum {
    BESVG_LINE_JOIN_MITER           = 0,
    BESVG_LINE_JOIN_ROUND           = 1,
    BESVG_LINE_JOIN_BEVEL           = 2,
} BESVG_LINE_JOIN_T;

typedef enum {
    BESVG_BLEND_NONE                = 0,
    BESVG_BLEND_SRC_OVER            = 1,
    BESVG_BLEND_DST_OVER            = 2,
    BESVG_BLEND_SRC_IN              = 3,
    BESVG_BLEND_DST_IN              = 4,
    BESVG_BLEND_MULTIPLY            = 5,
    BESVG_BLEND_ADDITIVE            = 9,
    BESVG_BLEND_SUBTRACT            = 10,
    BESVG_BLEND_NORMAL_LVGL         = 11,
    BESVG_BLEND_ADDITIVE_LVGL       = 12,
    BESVG_BLEND_SUBTRACT_LVGL       = 13,
    BESVG_BLEND_MULTIPLY_LVGL       = 14,
} BESVG_BLEND_T;

typedef struct {
    int32_t x;
    int32_t y;
} BESVG_POINT_T;

int besvg_set_scissor(int x, int y, int w, int h);

int besvg_clear_scissor(void);

int besvg_set_fill_rule(BESVG_FILL_RULE_T rule);

int besvg_set_line_cap(BESVG_LINE_CAP_T line_cap);

int besvg_set_line_join(BESVG_LINE_JOIN_T line_join);

int besvg_set_blend_mode(BESVG_BLEND_T blend);

int besvg_set_miter_limit(int miter_limit);

int besvg_set_bounding_box(float box[4]);

int besvg_reset_matrix(void);

int besvg_set_scale(float scale_x, float scale_y);

int besvg_set_rotate(float angle);

int besvg_set_translate(int32_t x, int32_t y);

int besvg_set_matrix(float m[3][3]);

int besvg_set_render_buf(void *buf, int w, int h, int stride, BESVG_FORMAT_T format,
                         bool tiled);

int besvg_fill_bgcolor(uint32_t color);

int besvg_wait_finish(void);

int besvg_draw_poly_lines(BESVG_POINT_T *points, int point_num, uint32_t line_weight,
                          uint32_t color);

int besvg_draw_dash_lines(BESVG_POINT_T *points, int point_num, uint32_t line_weight,
                          uint32_t color, int dash_array[2]);

int besvg_draw_polygon(BESVG_POINT_T *points, int point_num, uint32_t color);

#endif
