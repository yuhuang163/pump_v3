
/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_vglite.h"

#if LV_USE_DRAW_BES_VGLITE && LV_USE_VECTOR_GRAPHIC
#include "src/draw/lv_draw_vector_private.h"
#include "src/draw/lv_image_decoder_private.h"
#include "lv_vglite_buf.h"
#include "lv_vglite_path.h"
#include "lv_vglite_utils.h"

/*********************
 *      DEFINES
 *********************/
#define MAX_BOUND   9999

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

static void _lv_area_to_vglite(vg_lite_rectangle_t *rect, const lv_area_t *area)
{
    rect->x = area->x1;
    rect->y = area->y1;
    rect->width = lv_area_get_width(area);
    rect->height = lv_area_get_height(area);
}

static uint32_t _lv_color_to_vglite(const lv_color32_t *c, lv_opa_t opa)
{
    uint8_t r = c->red;
    uint8_t g = c->green;
    uint8_t b = c->blue;
    uint8_t a = LV_OPA_MIX2(c->alpha, opa);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

static void _lv_matrix_to_vglite(vg_lite_matrix_t *dst, const lv_matrix_t *src)
{
    lv_memcpy(dst, src, sizeof(lv_matrix_t));
}

static int _lv_convert_path_data(vg_lite_path_t *vgpath, const lv_vector_path_t *p)
{
    int ret = VG_LITE_SUCCESS;
    uint32_t pidx = 0;
    uint8_t *opcode = NULL;
    uint32_t opcode_sz = 0;
    float *opdata = NULL;
    uint32_t opdata_sz = 0;
    float start_x = 0;
    float start_y = 0;

    lv_vector_path_op_t *op = lv_array_front(&p->ops);
    uint32_t size = lv_array_size(&p->ops);
    for(uint32_t i = 0; i < size; i++) {
        if (op[i] == LV_VECTOR_PATH_OP_QUAD_TO)
            opdata_sz += sizeof(float) * 6;
        else if (op[i] == LV_VECTOR_PATH_OP_CUBIC_TO)
            opdata_sz += sizeof(float) * 6;
        else
            opdata_sz += sizeof(float) * 2;
        opcode_sz++;
    }
    if (opcode_sz == 0)
        return -1;

    opcode = lv_malloc(opcode_sz + 1);
    opdata = lv_malloc(opdata_sz);
    if (!opcode || !opdata) {
        LV_LOG_ERROR("%s: failed to malloc memory.", __func__);
        ret = -1;
        goto exit;
    }

    opcode_sz = 0;
    opdata_sz = 0;
    for(uint32_t i = 0; i < size; i++) {
        switch(op[i]) {
            case LV_VECTOR_PATH_OP_MOVE_TO: {
                    lv_fpoint_t *pt = lv_array_at(&p->points, pidx);
                    opcode[opcode_sz++] = VLC_OP_MOVE;
                    opdata[opdata_sz++] = (float)pt->x;
                    opdata[opdata_sz++] = (float)pt->y;
                    start_x = (float)pt->x;
                    start_y = (float)pt->y;
                    pidx += 1;
                }
                break;
            case LV_VECTOR_PATH_OP_LINE_TO: {
                    lv_fpoint_t *pt = lv_array_at(&p->points, pidx);
                    opcode[opcode_sz++] = VLC_OP_LINE;
                    opdata[opdata_sz++] = (float)pt->x;
                    opdata[opdata_sz++] = (float)pt->y;
                    pidx += 1;
                }
                break;
            case LV_VECTOR_PATH_OP_QUAD_TO: {
                    lv_fpoint_t *pt1 = lv_array_at(&p->points, pidx);
                    lv_fpoint_t *pt2 = lv_array_at(&p->points, pidx + 1);

                    lv_fpoint_t *last_pt = lv_array_at(&p->points, pidx - 1);

                    lv_fpoint_t cp[2];
                    cp[0].x = (last_pt->x + 2 * pt1->x) * (1.0f / 3.0f);
                    cp[0].y = (last_pt->y + 2 * pt1->y) * (1.0f / 3.0f);
                    cp[1].x = (pt2->x + 2 * pt1->x) * (1.0f / 3.0f);
                    cp[1].y = (pt2->y + 2 * pt1->y) * (1.0f / 3.0f);

                    opcode[opcode_sz++] = VLC_OP_CUBIC;
                    opdata[opdata_sz++] = (float)cp[0].x;
                    opdata[opdata_sz++] = (float)cp[0].y;
                    opdata[opdata_sz++] = (float)cp[1].x;
                    opdata[opdata_sz++] = (float)cp[1].y;
                    opdata[opdata_sz++] = (float)pt2->x;
                    opdata[opdata_sz++] = (float)pt2->y;
                    pidx += 2;
                }
                break;
            case LV_VECTOR_PATH_OP_CUBIC_TO: {
                    lv_fpoint_t *pt1 = lv_array_at(&p->points, pidx);
                    lv_fpoint_t *pt2 = lv_array_at(&p->points, pidx + 1);
                    lv_fpoint_t *pt3 = lv_array_at(&p->points, pidx + 2);

                    opcode[opcode_sz++] = VLC_OP_CUBIC;
                    opdata[opdata_sz++] = (float)pt1->x;
                    opdata[opdata_sz++] = (float)pt1->y;
                    opdata[opdata_sz++] = (float)pt2->x;
                    opdata[opdata_sz++] = (float)pt2->y;
                    opdata[opdata_sz++] = (float)pt3->x;
                    opdata[opdata_sz++] = (float)pt3->y;
                    pidx += 3;
                }
                break;
            case LV_VECTOR_PATH_OP_CLOSE: {
                    opcode[opcode_sz++] = VLC_OP_LINE;
                    opdata[opdata_sz++] = (float)start_x;
                    opdata[opdata_sz++] = (float)start_y;
                }
                break;
        }
    }

    vg_lite_init_path(vgpath, VG_LITE_FP32, VG_LITE_MEDIUM, 0, NULL, 0, 0, 0, 0);
    ret = vg_lite_append_path(vgpath, opcode, opdata, opcode_sz);

    /* Fix bounding_box for VLC_OP_QUAD/CUBIC control point */
    vgpath->bounding_box[0] = -MAX_BOUND;
    vgpath->bounding_box[1] = -MAX_BOUND;
    vgpath->bounding_box[2] = MAX_BOUND;
    vgpath->bounding_box[3] = MAX_BOUND;

exit:
    if (opcode)
        lv_free(opcode);
    if (opdata)
        lv_free(opdata);
    return ret;
}

static vg_lite_cap_style_t _lv_stroke_cap_to_vglite(lv_vector_stroke_cap_t cap)
{
    switch(cap) {
        case LV_VECTOR_STROKE_CAP_SQUARE:
            return VG_LITE_CAP_SQUARE;
        case LV_VECTOR_STROKE_CAP_ROUND:
            return VG_LITE_CAP_ROUND;
        case LV_VECTOR_STROKE_CAP_BUTT:
            return VG_LITE_CAP_BUTT;
        default:
            return VG_LITE_CAP_SQUARE;
    }
}

static vg_lite_join_style_t _lv_stroke_join_to_vglite(lv_vector_stroke_join_t join)
{
    switch(join) {
        case LV_VECTOR_STROKE_JOIN_BEVEL:
            return VG_LITE_JOIN_BEVEL;
        case LV_VECTOR_STROKE_JOIN_ROUND:
            return VG_LITE_JOIN_ROUND;
        case LV_VECTOR_STROKE_JOIN_MITER:
            return VG_LITE_JOIN_MITER;
        default:
            return VG_LITE_JOIN_BEVEL;
    }
}

static vg_lite_gradient_spreadmode_t _lv_spread_to_vglite(lv_vector_gradient_spread_t sp)
{
    switch(sp) {
        case LV_VECTOR_GRADIENT_SPREAD_PAD:
            return VG_LITE_GRADIENT_SPREAD_PAD;
        case LV_VECTOR_GRADIENT_SPREAD_REPEAT:
            return VG_LITE_GRADIENT_SPREAD_REPEAT;
        case LV_VECTOR_GRADIENT_SPREAD_REFLECT:
            return VG_LITE_GRADIENT_SPREAD_REFLECT;
        default:
            return VG_LITE_GRADIENT_SPREAD_PAD;
    }
}

static vg_lite_fill_t _lv_fill_rule_to_vglite(lv_vector_fill_t rule)
{
    switch(rule) {
        case LV_VECTOR_FILL_NONZERO:
            return VG_LITE_FILL_NON_ZERO;
        case LV_VECTOR_FILL_EVENODD:
            return VG_LITE_FILL_EVEN_ODD;
        default:
            return VG_LITE_FILL_NON_ZERO;
    }
}

static vg_lite_blend_t _lv_blend_to_vglite(lv_vector_blend_t blend)
{
    switch(blend) {
        case LV_VECTOR_BLEND_SRC_OVER:
            return VG_LITE_BLEND_SRC_OVER;
        case LV_VECTOR_BLEND_SCREEN:
            return VG_LITE_BLEND_SCREEN;
        case LV_VECTOR_BLEND_MULTIPLY:
            return VG_LITE_BLEND_MULTIPLY;
        case LV_VECTOR_BLEND_NONE:
            return VG_LITE_BLEND_NONE;
        case LV_VECTOR_BLEND_ADDITIVE:
            return VG_LITE_BLEND_ADDITIVE;
        case LV_VECTOR_BLEND_SRC_IN:
            return VG_LITE_BLEND_SRC_IN;
        case LV_VECTOR_BLEND_DST_OVER:
            return VG_LITE_BLEND_DST_OVER;
        case LV_VECTOR_BLEND_DST_IN:
            return VG_LITE_BLEND_DST_IN;
        case LV_VECTOR_BLEND_SUBTRACTIVE:
            return VG_LITE_BLEND_SUBTRACT;
        default:
            return VG_LITE_BLEND_SRC_OVER;
    }
}

static int _vglite_load_image(vg_lite_buffer_t *srcbuf, const lv_draw_image_dsc_t *p)
{
    lv_image_decoder_dsc_t decoder_dsc;
    lv_image_decoder_args_t args = { 0 };
    lv_result_t res = lv_image_decoder_open(&decoder_dsc, p->src, &args);
    if (res != LV_RESULT_OK) {
        LV_LOG_ERROR("Failed to open image");
        return -1;
    }

    if (!decoder_dsc.decoded) {
        lv_image_decoder_close(&decoder_dsc);
        LV_LOG_ERROR("Image not ready");
        return -1;
    }

    const uint8_t *src_buf = decoder_dsc.decoded->data;
    const lv_image_header_t *header = &decoder_dsc.decoded->header;

    vglite_set_src_buf((uint32_t *)src_buf, header->w, header->h, header->stride, header->cf);
    lv_image_decoder_close(&decoder_dsc);
    return 0;
}

static void _vglite_draw_path(const lv_vector_fill_dsc_t *dsc, vg_lite_path_t *path,
                               vg_lite_matrix_t *matrix, vg_lite_blend_t blend)
{
    vg_lite_buffer_t *vgbuf = vglite_get_dest_buf();
    vg_lite_fill_t fill_rule = _lv_fill_rule_to_vglite(dsc->fill_rule);
    uint32_t color = _lv_color_to_vglite(&dsc->color, dsc->opa);

    if (dsc->style == LV_VECTOR_DRAW_STYLE_SOLID) {
        VGLITE_CHECK_ERROR(vg_lite_draw(vgbuf, path, fill_rule, matrix, blend, color));

    } else if (dsc->style == LV_VECTOR_DRAW_STYLE_PATTERN) {
        vg_lite_buffer_t *srcbuf = vglite_get_src_buf();

        if (_vglite_load_image(srcbuf, &dsc->img_dsc))
            return;

        VGLITE_CHECK_ERROR(vg_lite_draw_pattern(vgbuf, path, fill_rule, matrix, srcbuf, matrix,
                            blend, VG_LITE_PATTERN_COLOR, 0, color, VG_LITE_FILTER_LINEAR));

    } else if (dsc->style == LV_VECTOR_DRAW_STYLE_GRADIENT) {
        lv_vector_gradient_style_t style = dsc->gradient.style;
        vg_lite_gradient_spreadmode_t spreadmode = _lv_spread_to_vglite(dsc->gradient.spread);

        LV_UNUSED(spreadmode);

        if (style == LV_VECTOR_GRADIENT_STYLE_LINEAR) {
            vg_lite_linear_gradient_t gradient;
            vg_lite_matrix_t *grad_matrix;

            vg_lite_uint32_t colors[LV_GRADIENT_MAX_STOPS];
            vg_lite_uint32_t stops[LV_GRADIENT_MAX_STOPS];
            lv_color32_t col32[LV_GRADIENT_MAX_STOPS];
            lv_opa_t opa;

            /* Gradient setup */
            vg_lite_uint32_t cnt = LV_MAX(dsc->gradient.stops_count, LV_GRADIENT_MAX_STOPS);

            for (uint8_t i = 0; i < cnt; i++) {
                stops[i] = dsc->gradient.stops[i].frac;

                opa = LV_OPA_MIX2(dsc->gradient.stops[i].opa, dsc->opa);

                col32[i] = lv_color_to_32(dsc->gradient.stops[i].color, opa);
                colors[i] = vglite_get_color(col32[i], true);
            }

            lv_memzero(&gradient, sizeof(vg_lite_linear_gradient_t));

            VGLITE_CHECK_ERROR(vg_lite_init_grad(&gradient));
            VGLITE_CHECK_ERROR(vg_lite_set_grad(&gradient, cnt, colors, stops));
            VGLITE_CHECK_ERROR(vg_lite_update_grad(&gradient));

            grad_matrix = vg_lite_get_grad_matrix(&gradient);
            lv_memcpy(grad_matrix, matrix, sizeof(vg_lite_matrix_t));

            VGLITE_CHECK_ERROR(vg_lite_draw_gradient(vgbuf, path, fill_rule, matrix, &gradient,
                                                     blend));
            vg_lite_clear_grad(&gradient);

        } else if (style == LV_VECTOR_GRADIENT_STYLE_RADIAL &&
                   vg_lite_query_feature(gcFEATURE_BIT_VG_RADIAL_GRADIENT)) {
            /* FIXME */
        }
    }
}

#define DASH_MAX_COUNT          16
static void _vglite_set_stroke(const lv_vector_stroke_dsc_t *dsc, vg_lite_path_t *path)
{
    if (dsc->opa == LV_OPA_0)
        return;

    uint32_t color = _lv_color_to_vglite(&dsc->color, dsc->opa);
    uint32_t line_width = (uint32_t)dsc->width;
    uint32_t miter_limit = (uint32_t)dsc->miter_limit;
    vg_lite_cap_style_t line_cap = _lv_stroke_cap_to_vglite(dsc->cap);
    vg_lite_join_style_t line_join = _lv_stroke_join_to_vglite(dsc->join);
    vg_lite_float_t dash[DASH_MAX_COUNT] = {0};
    uint32_t dash_count = 0;

    if (!lv_array_is_empty(&dsc->dash_pattern)) {
        float *dash_array = lv_array_front(&dsc->dash_pattern);
        for (int i = 0; i <  dsc->dash_pattern.size && i < DASH_MAX_COUNT; i++) {
            dash[i] = dash_array[i];
            dash_count++;
        }
    }

    VGLITE_CHECK_ERROR(vg_lite_set_stroke(path, line_cap, line_join, line_width, miter_limit,
                                        dash, dash_count, 0, color));
    VGLITE_CHECK_ERROR(vg_lite_update_stroke(path));
}

static void _task_draw_cb(void *ctx, const lv_vector_path_t *path, const lv_vector_path_ctx_t *dsc)
{
    LV_UNUSED(ctx);

    if (!path) {  /* clear */
        vg_lite_buffer_t *vgbuf = vglite_get_dest_buf();
        vg_lite_rectangle_t r;
        _lv_area_to_vglite(&r, &dsc->scissor_area);

        uint32_t c = _lv_color_to_vglite(&dsc->fill_dsc.color, dsc->fill_dsc.opa);

        vg_lite_clear(vgbuf, &r, c);
        return;
    }

    lv_opa_t fill_opa = dsc->fill_dsc.opa;
    lv_opa_t stroke_opa = dsc->stroke_dsc.opa;
    vg_lite_blend_t blend = _lv_blend_to_vglite(dsc->blend_mode);
    vg_lite_matrix_t matrix;
    vg_lite_path_t vgpath;

    if (_lv_convert_path_data(&vgpath, path) != VG_LITE_SUCCESS) {
        LV_LOG_ERROR("%s: Failed to convert path data.", __func__);
        return;
    }

    _lv_matrix_to_vglite(&matrix, &dsc->matrix);

    if (stroke_opa > LV_OPA_0) {
        _vglite_set_stroke(&dsc->stroke_dsc, &vgpath);
        if (fill_opa == 0)
            vg_lite_set_path_type(&vgpath, VG_LITE_DRAW_STROKE_PATH);
        else
            vg_lite_set_path_type(&vgpath, VG_LITE_DRAW_FILL_STROKE_PATH);
    }

    _vglite_draw_path(&dsc->fill_dsc, &vgpath, &matrix, blend);

    vg_lite_clear_path(&vgpath);
    vglite_run();
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_draw_vglite_vector(lv_draw_task_t * t, const lv_draw_vector_dsc_t *dsc)
{
    LV_UNUSED(t);

    if(dsc->task_list == NULL)
        return;

    lv_layer_t *layer = dsc->base.layer;
    lv_draw_buf_t *draw_buf = layer->draw_buf;
    if(draw_buf == NULL)
        return;

    LV_PROFILER_DRAW_BEGIN;

    lv_ll_t *task_list = dsc->task_list;
    lv_vector_for_each_destroy_tasks(task_list, _task_draw_cb, NULL);

    LV_PROFILER_DRAW_END;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* LV_USE_DRAW_BES_VGLITE && LV_USE_VECTOR_GRAPHIC */
