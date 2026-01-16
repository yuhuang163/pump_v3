/**
 * @file lv_draw_vglite_line.c
 *
 */

/**
 * Copyright 2022-2024 NXP
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
#include "math.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**
 * Draw line shape with effects
 *
 * @param[in] point1 Starting point with relative coordinates
 * @param[in] point2 Ending point with relative coordinates
 * @param[in] clip_area Clip area with relative coordinates to dest buff
 * @param[in] dsc Line description structure (width, rounded ending, opacity, ...)
 *
 */
static void _vglite_draw_stroke_line(const lv_point_t * point1, const lv_point_t * point2,
                              const lv_area_t * clip_area, const lv_draw_line_dsc_t * dsc);
static void _vglite_draw_rect_line(const lv_point_t * point1, const lv_point_t * point2,
                              const lv_area_t * clip_area, const lv_draw_line_dsc_t * dsc);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_vglite_line(lv_draw_task_t * t, const lv_draw_line_dsc_t * dsc)
{
    if(dsc->width == 0)
        return;
    if(dsc->opa <= (lv_opa_t)LV_OPA_MIN)
        return;
    if(dsc->p1.x == dsc->p2.x && dsc->p1.y == dsc->p2.y)
        return;

    lv_layer_t * layer = t->target_layer;
    lv_area_t clip_area;
    clip_area.x1 = LV_MIN(dsc->p1.x, dsc->p2.x) - dsc->width / 2;
    clip_area.x2 = LV_MAX(dsc->p1.x, dsc->p2.x) + dsc->width / 2;
    clip_area.y1 = LV_MIN(dsc->p1.y, dsc->p2.y) - dsc->width / 2;
    clip_area.y2 = LV_MAX(dsc->p1.y, dsc->p2.y) + dsc->width / 2;
    if(!lv_area_intersect(&clip_area, &clip_area, &t->clip_area))
        return; /*Fully clipped, nothing to do*/
    lv_area_move(&clip_area, -layer->buf_area.x1, -layer->buf_area.y1);

    lv_point_t point1 = {dsc->p1.x - layer->buf_area.x1, dsc->p1.y - layer->buf_area.y1};
    lv_point_t point2 = {dsc->p2.x - layer->buf_area.x1, dsc->p2.y - layer->buf_area.y1};

    bool is_dashed = (dsc->dash_width && dsc->dash_gap);

    LV_PROFILER_DRAW_BEGIN;

    if (is_dashed)
        _vglite_draw_stroke_line(&point1, &point2, &clip_area, dsc);
    else
        _vglite_draw_rect_line(&point1, &point2, &clip_area, dsc);

    LV_PROFILER_DRAW_END;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void _vglite_draw_stroke_line(const lv_point_t * point1, const lv_point_t * point2,
                              const lv_area_t * clip_area, const lv_draw_line_dsc_t * dsc)
{
    vg_lite_path_t path;
    vg_lite_buffer_t * vgbuf = vglite_get_dest_buf();
    vg_lite_cap_style_t cap_style = (dsc->round_start || dsc->round_end) ? VG_LITE_CAP_ROUND : VG_LITE_CAP_BUTT;
    vg_lite_join_style_t join_style = (dsc->round_start || dsc->round_end) ? VG_LITE_JOIN_ROUND : VG_LITE_JOIN_MITER;

    bool is_dashed = (dsc->dash_width && dsc->dash_gap);

    vg_lite_float_t stroke_dash_pattern[2] = {0, 0};
    uint32_t stroke_dash_count = 0;
    vg_lite_float_t stroke_dash_phase = 0;
    if(is_dashed) {
        stroke_dash_pattern[0] = (vg_lite_float_t)dsc->dash_width;
        stroke_dash_pattern[1] = (vg_lite_float_t)dsc->dash_gap;
        stroke_dash_count = sizeof(stroke_dash_pattern) / sizeof(vg_lite_float_t);
        stroke_dash_phase = (vg_lite_float_t)dsc->dash_width / 2;
    }

    /*** Init path ***/
    int32_t width = dsc->width;
    uint8_t opcode[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE,
        VLC_OP_END,
    };
    float opdata[] = {
        point1->x, point1->y,
        point2->x, point2->y,
    };

    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0, 0, 0, 0);

    VGLITE_CHECK_ERROR(vg_lite_append_path(&path, opcode, opdata, sizeof(opcode)));

    lv_color32_t col32 = lv_color_to_32(dsc->color, dsc->opa);
    vg_lite_color_t vgcol = vglite_get_color(col32, false);

    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);

    /*** Draw line ***/
    VGLITE_CHECK_ERROR(vg_lite_set_stroke(&path, cap_style, join_style, width, 8, stroke_dash_pattern, stroke_dash_count,
                                          stroke_dash_phase, vgcol));

    VGLITE_CHECK_ERROR(vg_lite_update_stroke(&path));

    VGLITE_CHECK_ERROR(vg_lite_set_draw_path_type(&path, VG_LITE_DRAW_STROKE_PATH));

    VGLITE_CHECK_ERROR(vg_lite_draw(vgbuf, &path, VG_LITE_FILL_NON_ZERO, &matrix, VG_LITE_BLEND_SRC_OVER, vgcol));

    vglite_run();

    VGLITE_CHECK_ERROR(vg_lite_clear_path(&path));
}

static void _vglite_draw_rect_line(const lv_point_t * point1, const lv_point_t * point2,
                              const lv_area_t * clip_area, const lv_draw_line_dsc_t * dsc)
{
    vg_lite_path_t path;
    vg_lite_buffer_t * vgbuf = vglite_get_dest_buf();

    /*** Init path ***/
    int32_t width = dsc->width;
    float x1 = point1->x * 1.0f;
    float y1 = point1->y * 1.0f;
    float x2 = point2->x * 1.0f;
    float y2 = point2->y * 1.0f;
    uint8_t opcode[6];
    float opdata[14];
    int i = 0, j = 0;
    float rx1, ry1, rx2, ry2;
    float rx3, ry3, rx4, ry4;
    float cx1, cy1, cx2, cy2;
    float w = width / 2.0f;
    float d = sqrtf(powf(y1-y2, 2) + powf(x1-x2, 2));

    rx1 = x1 - w * (y1 - y2) / d;
    ry1 = y1 - w * (x2 - x1) / d;
    rx2 = x2 - w * (y1 - y2) / d;
    ry2 = y2 - w * (x2 - x1) / d;
    rx3 = x2 + w * (y1 - y2) / d;
    ry3 = y2 + w * (x2 - x1) / d;
    rx4 = x1 + w * (y1 - y2) / d;
    ry4 = y1 + w * (x2 - x1) / d;

    cx1 = x1 + (w*2/d) * (x1-x2);
    cy1 = y1 + (w*2/d) * (y1-y2);
    cx2 = x2 + (w*2/d) * (x2-x1);
    cy2 = y2 + (w*2/d) * (y2-y1);

    opcode[i++] = VLC_OP_MOVE;
    opdata[j++] = rx1;
    opdata[j++] = ry1;

    opcode[i++] = VLC_OP_LINE;
    opdata[j++] = rx2;
    opdata[j++] = ry2;

    if (dsc->round_end) {
        opcode[i++] = VLC_OP_QUAD;
        opdata[j++] = cx2;
        opdata[j++] = cy2;
        opdata[j++] = rx3;
        opdata[j++] = ry3;
    } else {
        opcode[i++] = VLC_OP_LINE;
        opdata[j++] = rx3;
        opdata[j++] = ry3;
    }

    opcode[i++] = VLC_OP_LINE;
    opdata[j++] = rx4;
    opdata[j++] = ry4;

    if (dsc->round_start) {
        opcode[i++] = VLC_OP_QUAD;
        opdata[j++] = cx1;
        opdata[j++] = cy1;
        opdata[j++] = rx1;
        opdata[j++] = ry1;
    } else {
        opcode[i++] = VLC_OP_LINE;
        opdata[j++] = rx1;
        opdata[j++] = ry1;
    }

    opcode[i++] = VLC_OP_END;

    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0, 0, 0, 0);
    VGLITE_CHECK_ERROR(vg_lite_append_path(&path, opcode, opdata, i));

    /* Fix bounding_box for VLC_OP_QUAD control point */
    path.bounding_box[0] = -vgbuf->width;
    path.bounding_box[1] = -vgbuf->height;
    path.bounding_box[2] = vgbuf->width*2;
    path.bounding_box[3] = vgbuf->height*2;

    lv_color32_t col32 = lv_color_to_32(dsc->color, dsc->opa);
    vg_lite_color_t vgcol = vglite_get_color(col32, false);

    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);

    /*** Draw line ***/
    VGLITE_CHECK_ERROR(vg_lite_draw(vgbuf, &path, VG_LITE_FILL_NON_ZERO, &matrix, VG_LITE_BLEND_SRC_OVER, vgcol));

    vglite_run();

    VGLITE_CHECK_ERROR(vg_lite_clear_path(&path));
}

#endif /*LV_USE_DRAW_BES_VGLITE*/
