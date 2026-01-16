/**
 * @file lv_vglite_freetype.c
 *
 */

/**
 * Copyright 2025 BES
 *
 * SPDX-License-Identifier: MIT
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_vglite_freetype.h"

#if LV_USE_DRAW_BES_VGLITE && LV_USE_FREETYPE
#include "src/draw/lv_draw_label_private.h"
#include "src/libs/freetype/lv_freetype_private.h"
#include "lv_vglite_buf.h"
#include "lv_vglite_matrix.h"
#include "lv_vglite_utils.h"

/*********************
 *      DEFINES
 *********************/
#define MAX_BOUND           20000
#define MAX_DATA_COUNT      1024
#define FT_SCALE(x)         (LV_FREETYPE_F26DOT6_TO_FLOAT(x)/64.0f)

/**********************
 *      TYPEDEFS
 **********************/
struct ft2_path_t {
    int16_t data[MAX_DATA_COUNT];
    int count;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void freetype_outline_cb(lv_event_t *event);
static void freetype_push_path_data(const lv_freetype_outline_event_param_t * param);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void vglite_freetype_init_outline_cb(void)
{
    lv_freetype_outline_add_event(freetype_outline_cb, LV_EVENT_ALL, NULL);
}

int vglite_freetype_draw(lv_draw_task_t * t, lv_draw_glyph_dsc_t *dsc)
{
    int error;
    static vg_lite_path_t path;
    vg_lite_buffer_t *dstbuf = vglite_get_dest_buf();

    lv_point_t pos = { dsc->letter_coords->x1, dsc->letter_coords->y1 };
    lv_color32_t col32 = lv_color_to_32(dsc->color, dsc->opa);
    uint32_t color = vglite_get_color(col32, false);
    struct ft2_path_t *ftpath = (struct ft2_path_t *)dsc->glyph_data;
    int16_t *path_data = ftpath->data;
    int path_count = ftpath->count;

    if (!ftpath)
        return VG_LITE_INVALID_ARGUMENT;

    lv_memzero(&path, sizeof(path));
    path_data[path_count] = VLC_OP_END;
    path_count++;
    vg_lite_init_path(&path, VG_LITE_S16, VG_LITE_MEDIUM, path_count * sizeof(int16_t),
                (void *)path_data, -MAX_BOUND, -MAX_BOUND, MAX_BOUND, MAX_BOUND);

    /* transform matrix */
    float scale = FT_SCALE(lv_freetype_outline_get_scale(dsc->g->resolved_font));
    vg_lite_matrix_t matrix;

    vg_lite_identity(&matrix);
    vg_lite_translate(pos.x - dsc->g->ofs_x, pos.y + dsc->g->box_h + dsc->g->ofs_y, &matrix);
    vg_lite_scale(scale, scale, &matrix);

    /* flip y pos */
    matrix.m[1][1] = -matrix.m[1][1];

    /* move to relative pos of layer */
    lv_layer_t *layer = t->target_layer;
    vg_lite_translate(-layer->buf_area.x1 / scale, layer->buf_area.y1 / scale, &matrix);

    error = vg_lite_draw(dstbuf, &path, VG_LITE_FILL_NON_ZERO, &matrix,
                        VG_LITE_BLEND_SRC_OVER, color);
    vg_lite_clear_path(&path);
    return error;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void freetype_push_path_data(const lv_freetype_outline_event_param_t *param)
{
    struct ft2_path_t *ftpath = (struct ft2_path_t *)param->outline;
    lv_freetype_outline_type_t type = param->type;
    int16_t *path_data = ftpath->data;
    int i = ftpath->count;

    switch (type) {
        case LV_FREETYPE_OUTLINE_MOVE_TO:
            if ((i + 4) < MAX_DATA_COUNT) {
                path_data[i + 0] = VLC_OP_MOVE;
                path_data[i + 1] = (int16_t)(param->to.x);
                path_data[i + 2] = (int16_t)(param->to.y);
                ftpath->count += 3;
                LV_LOG_INFO("MOVE_TO:  %d %d", path_data[i + 1], path_data[i + 2]);
            }
            break;
        case LV_FREETYPE_OUTLINE_LINE_TO:
            if ((i + 4) < MAX_DATA_COUNT) {
                path_data[i + 0] = VLC_OP_LINE;
                path_data[i + 1] = (int16_t)(param->to.x);
                path_data[i + 2] = (int16_t)(param->to.y);
                ftpath->count += 3;
                LV_LOG_INFO("LINE_TO:  %d %d", path_data[i + 1], path_data[i + 2]);
            }
            break;
        case LV_FREETYPE_OUTLINE_CONIC_TO:
            if ((i + 6) < MAX_DATA_COUNT) {
                path_data[i + 0] = VLC_OP_QUAD;
                path_data[i + 1] = (int16_t)(param->control1.x);
                path_data[i + 2] = (int16_t)(param->control1.y);
                path_data[i + 3] = (int16_t)(param->to.x);
                path_data[i + 4] = (int16_t)(param->to.y);
                ftpath->count += 5;
                LV_LOG_INFO("QUAD_TO:  %d %d %d %d",
                            path_data[i + 1], path_data[i + 2],
                            path_data[i + 3], path_data[i + 4]);
            }
            break;
        case LV_FREETYPE_OUTLINE_CUBIC_TO:
            if ((i + 8) < MAX_DATA_COUNT) {
                path_data[i + 0] = VLC_OP_QUAD;
                path_data[i + 1] = (int16_t)(param->control1.x);
                path_data[i + 2] = (int16_t)(param->control1.y);
                path_data[i + 3] = (int16_t)(param->control2.x);
                path_data[i + 4] = (int16_t)(param->control2.y);
                path_data[i + 5] = (int16_t)(param->to.x);
                path_data[i + 6] = (int16_t)(param->to.y);
                ftpath->count += 7;
                LV_LOG_INFO("CUBIC_TO:  %d %d %d %d %d %d",
                            path_data[i + 1], path_data[i + 2],
                            path_data[i + 3], path_data[i + 4],
                            path_data[i + 5], path_data[i + 6]);
            }
            break;
        case LV_FREETYPE_OUTLINE_END:
            break;
        default:
            LV_LOG_ERROR("unknow freetype param type: %d", type);
            break;
    }
}

static void freetype_outline_cb(lv_event_t *event)
{
    lv_freetype_outline_event_param_t *param = lv_event_get_param(event);
    lv_event_code_t code = lv_event_get_code(event);
    struct ft2_path_t *p;
    switch (code) {
        case LV_EVENT_CREATE:
            p = lv_malloc_zeroed(sizeof(struct ft2_path_t));
            param->outline = p;
            break;
        case LV_EVENT_DELETE:
            if (param->outline)
                lv_free(param->outline);
            break;
        case LV_EVENT_INSERT:
            freetype_push_path_data(param);
            break;
        default:
            LV_LOG_WARN("unkown freetype event code: %d", code);
            break;
    }
}

#endif /* LV_USE_DRAW_BES_VGLITE && LV_USE_FREETYPE */
