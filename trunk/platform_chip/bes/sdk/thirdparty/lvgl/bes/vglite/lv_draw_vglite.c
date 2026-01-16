/**
 * @file lv_draw_vglite.c
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
#include "lv_vglite_utils.h"
#include "src/core/lv_global.h"

/*********************
 *      DEFINES
 *********************/

#define DRAW_UNIT_ID_VGLITE 2

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/

/*
 * Evaluate a task and set the score and preferred VGLite draw unit.
 * Return 1 if task is preferred, 0 otherwise (task is not supported).
 */
static int32_t _vglite_evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task);

/*
 * Dispatch (assign) a task to VGLite draw unit (itself).
 * Return 1 if task was dispatched, 0 otherwise (task not supported).
 */
static int32_t _vglite_dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer);

/*
 * Delete the VGLite draw unit.
 */
static int32_t _vglite_delete(lv_draw_unit_t * draw_unit);

#if LV_USE_BES_VGLITE_DRAW_THREAD
    static void _vglite_render_thread_cb(void * ptr);
#endif

static void _vglite_execute_drawing(lv_draw_vglite_unit_t * u);

/**********************
 *  STATIC VARIABLES
 **********************/
#define _draw_info LV_GLOBAL_DEFAULT()->draw_info

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_vglite_init(void)
{
    lv_draw_buf_vglite_init_handlers();

    lv_draw_vglite_unit_t * draw_vglite_unit = lv_draw_create_unit(sizeof(lv_draw_vglite_unit_t));
    draw_vglite_unit->base_unit.evaluate_cb = _vglite_evaluate;
    draw_vglite_unit->base_unit.dispatch_cb = _vglite_dispatch;
    draw_vglite_unit->base_unit.delete_cb = _vglite_delete;
    draw_vglite_unit->base_unit.name = "BES_VGLITE";

#if LV_USE_BES_VGLITE_DRAW_THREAD
#if defined(LV_DRAW_THREAD_STACK_SIZE)
    uint32_t stack_size = LV_DRAW_THREAD_STACK_SIZE;
#elif defined(LV_DRAW_THREAD_STACKSIZE)
    uint32_t stack_size = LV_DRAW_THREAD_STACKSIZE;
#else
    uint32_t stack_size = 32768;
#endif
    lv_thread_init(&draw_vglite_unit->thread, "bes_vglite", LV_THREAD_PRIO_HIGH,
            _vglite_render_thread_cb, stack_size, draw_vglite_unit);
#endif
}

void lv_draw_vglite_deinit(void)
{
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline bool _vglite_src_cf_supported(lv_color_format_t cf)
{
    bool is_cf_supported = false;

    switch(cf) {
        case LV_COLOR_FORMAT_I1:
        case LV_COLOR_FORMAT_I2:
        case LV_COLOR_FORMAT_I4:
        case LV_COLOR_FORMAT_I8:
        case LV_COLOR_FORMAT_A4:
        case LV_COLOR_FORMAT_A8:
        case LV_COLOR_FORMAT_L8:
        case LV_COLOR_FORMAT_YUY2:
        case LV_COLOR_FORMAT_RGB565:
        case LV_COLOR_FORMAT_RGB565A8:
        case LV_COLOR_FORMAT_RGB888:
        case LV_COLOR_FORMAT_ARGB8888:
        case LV_COLOR_FORMAT_XRGB8888:
            is_cf_supported = true;
            break;
        default:
            break;
    }

    return is_cf_supported;
}

static inline bool _vglite_dest_cf_supported(lv_color_format_t cf)
{
    bool is_cf_supported = false;

    switch(cf) {
        case LV_COLOR_FORMAT_A8:
        case LV_COLOR_FORMAT_L8:
        case LV_COLOR_FORMAT_RGB565:
        case LV_COLOR_FORMAT_RGB565A8:
        case LV_COLOR_FORMAT_RGB888:
        case LV_COLOR_FORMAT_ARGB8888:
        case LV_COLOR_FORMAT_XRGB8888:
            is_cf_supported = true;
            break;
        default:
            break;
    }

    return is_cf_supported;
}

static int32_t _vglite_evaluate(lv_draw_unit_t * u, lv_draw_task_t * t)
{
#if LV_USE_BES_VGLITE_COMPRESS
    /* Compressed framebuffers only allow GPU drawing */
    t->preference_score = 80;
    t->preferred_draw_unit_id = DRAW_UNIT_ID_VGLITE;
    return 1;
#else
    int ret = 0;

    LV_UNUSED(u);

    const lv_draw_dsc_base_t * draw_dsc_base = (lv_draw_dsc_base_t *) t->draw_dsc;

    if (!_vglite_dest_cf_supported(draw_dsc_base->layer->color_format))
        return 0;

    switch(t->type) {
        case LV_DRAW_TASK_TYPE_FILL:
        case LV_DRAW_TASK_TYPE_LINE:
        case LV_DRAW_TASK_TYPE_ARC:
        case LV_DRAW_TASK_TYPE_TRIANGLE:
        case LV_DRAW_TASK_TYPE_LETTER:
        case LV_DRAW_TASK_TYPE_LABEL:
        case LV_DRAW_TASK_TYPE_BORDER:
        case LV_DRAW_TASK_TYPE_BOX_SHADOW:
        case LV_DRAW_TASK_TYPE_MASK_RECTANGLE:
#if LV_USE_VECTOR_GRAPHIC
        case LV_DRAW_TASK_TYPE_VECTOR:
#endif
            ret = 1;
            break;

        case LV_DRAW_TASK_TYPE_LAYER: {
            const lv_draw_image_dsc_t * draw_dsc = (lv_draw_image_dsc_t *) t->draw_dsc;
            lv_layer_t * layer_to_draw = (lv_layer_t *)draw_dsc->src;

            if (!_vglite_src_cf_supported(layer_to_draw->color_format))
                ret = 0;
            else
                ret = 1;
            break;
        }

        case LV_DRAW_TASK_TYPE_IMAGE: {
            const lv_draw_image_dsc_t * draw_dsc = (lv_draw_image_dsc_t *) t->draw_dsc;
            const void *src = (const void *)draw_dsc->src;
            lv_image_src_t src_type = lv_image_src_get_type((const void *)draw_dsc->src);
            bool need_decode = false;
            if (src_type == LV_IMAGE_SRC_VARIABLE) {
                const lv_image_dsc_t * img_dsc = src;

                if (img_dsc->header.cf == LV_COLOR_FORMAT_RAW) {
                    need_decode = true;
                } else {
                    if ((!_vglite_src_cf_supported(img_dsc->header.cf)) ||
                        (!vglite_buf_aligned(img_dsc->data, img_dsc->header.w, img_dsc->header.stride, img_dsc->header.cf)))
                        ret = 0;
                    else
                        ret = 1;
                }
            } else {
                need_decode = true;
            }
            if (need_decode) {
                lv_image_header_t header;
                if (lv_image_decoder_get_info(src, &header) == LV_RESULT_OK) {
                    if ((!_vglite_src_cf_supported(header.cf)) ||
                        (!vglite_buf_aligned(0, header.w, header.stride, header.cf)))
                        ret = 0;
                    else
                        ret = 1;
                } else {
                    ret = 0;
                }
            }
            break;
        }

        default:
            ret = 0;
            break;
    }

    if (ret == 1) {
        t->preference_score = 80;
        t->preferred_draw_unit_id = DRAW_UNIT_ID_VGLITE;
    }
    return ret;
#endif
}

static int32_t _vglite_dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer)
{
    lv_draw_vglite_unit_t * draw_vglite_unit = (lv_draw_vglite_unit_t *) draw_unit;

    /* Return immediately if it's busy with draw task. */
    if(draw_vglite_unit->task_act)
        return 0;

    /* Try to get an ready to draw. */
    lv_draw_task_t * t = lv_draw_get_next_available_task(layer, NULL, DRAW_UNIT_ID_VGLITE);
    if (t == NULL)
        return LV_DRAW_UNIT_IDLE;

    if (t->preferred_draw_unit_id != DRAW_UNIT_ID_VGLITE)
        return LV_DRAW_UNIT_IDLE;

    if (lv_draw_layer_alloc_buf(layer) == NULL)
        return LV_DRAW_UNIT_IDLE;

    /* remember draw unit for access to unit's context */
    t->draw_unit = (lv_draw_unit_t *)draw_unit;

    t->state = LV_DRAW_TASK_STATE_IN_PROGRESS;
    draw_vglite_unit->task_act = t;

#if LV_USE_BES_VGLITE_DRAW_THREAD
    /* Let the render thread work. */
    if(draw_vglite_unit->inited)
        lv_thread_sync_signal(&draw_vglite_unit->sync);
#else
    _vglite_execute_drawing(draw_vglite_unit);

    draw_vglite_unit->task_act->state = LV_DRAW_TASK_STATE_FINISHED;
    draw_vglite_unit->task_act = NULL;

    /* The draw unit is free now. Request a new dispatching as it can get a new task. */
    lv_draw_dispatch_request();
#endif

    return 1;
}

static int32_t _vglite_delete(lv_draw_unit_t * draw_unit)
{
#if LV_USE_BES_VGLITE_DRAW_THREAD
    lv_draw_vglite_unit_t * draw_vglite_unit = (lv_draw_vglite_unit_t *) draw_unit;

    LV_LOG_INFO("Cancel VGLite draw thread.");
    draw_vglite_unit->exit_status = true;

    if(draw_vglite_unit->inited)
        lv_thread_sync_signal(&draw_vglite_unit->sync);

    lv_result_t res = lv_thread_delete(&draw_vglite_unit->thread);

    return res;
#else
    LV_UNUSED(draw_unit);

    return 1;
#endif
}

static void _vglite_execute_drawing(lv_draw_vglite_unit_t * u)
{
    lv_draw_task_t * t = u->task_act;
    lv_layer_t * layer = t->target_layer;
    lv_draw_buf_t * draw_buf = layer->draw_buf;

    lv_area_t clip_area;
    lv_area_copy(&clip_area, &t->clip_area);
    lv_area_move(&clip_area, -layer->buf_area.x1, -layer->buf_area.y1);

    lv_area_t draw_area;
    lv_area_copy(&draw_area, &t->area);
    lv_area_move(&draw_area, -layer->buf_area.x1, -layer->buf_area.y1);

    if(!lv_area_intersect(&draw_area, &draw_area, &clip_area))
        return; /*Fully clipped, nothing to do*/

    /* Set target buffer */
    vglite_set_dest_buf(draw_buf->data, draw_buf->header.w, draw_buf->header.h, draw_buf->header.stride,
                        draw_buf->header.cf);

    /* Invalidate the drawing area */
	if(_draw_info.unit_cnt > 1)
        lv_draw_vglite_sync_invalidate(draw_buf->data, draw_buf->data_size);

    /* Set scissor area, excluding the split blit case */
    vglite_set_scissor(&clip_area);

    switch(t->type) {
        case LV_DRAW_TASK_TYPE_LETTER:
            lv_draw_vglite_letter(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_LABEL:
            lv_draw_vglite_label(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_FILL:
            lv_draw_vglite_fill(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_BORDER:
            lv_draw_vglite_border(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_BOX_SHADOW:
            lv_draw_vglite_box_shadow(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_IMAGE:
            lv_draw_vglite_img(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_ARC:
            lv_draw_vglite_arc(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_LINE:
            lv_draw_vglite_line(t, t->draw_dsc);
            break;
        case LV_DRAW_TASK_TYPE_LAYER:
            lv_draw_vglite_layer(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_TRIANGLE:
            lv_draw_vglite_triangle(t, t->draw_dsc);
            break;
        case LV_DRAW_TASK_TYPE_MASK_RECTANGLE:
            lv_draw_vglite_mask_rect(t, t->draw_dsc, &t->area);
            break;
#if LV_USE_VECTOR_GRAPHIC
        case LV_DRAW_TASK_TYPE_VECTOR:
            lv_draw_vglite_vector(t, t->draw_dsc);
            break;
#endif
        default:
            break;
    }

    /* Disable scissor */
    vglite_set_scissor(&layer->buf_area);
}


#if LV_USE_BES_VGLITE_DRAW_THREAD
static void _vglite_render_thread_cb(void * ptr)
{
    lv_draw_vglite_unit_t * u = ptr;

    lv_thread_sync_init(&u->sync);
    u->inited = true;

    while(1) {
        /* Wait for sync if there is no task set. */
        while(u->task_act == NULL
             ) {
            if(u->exit_status)
                break;

            lv_thread_sync_wait(&u->sync);
        }

        if(u->exit_status) {
            LV_LOG_INFO("Ready to exit VGLite draw thread.");
            break;
        }

        if(u->task_act) {
            _vglite_execute_drawing(u);
        }

        /* Signal the ready state to dispatcher. */
        u->task_act->state = LV_DRAW_TASK_STATE_FINISHED;

        /* Cleanup. */
        u->task_act = NULL;

        /* The draw unit is free now. Request a new dispatching as it can get a new task. */
        lv_draw_dispatch_request();
    }

    u->inited = false;
    lv_thread_sync_delete(&u->sync);
    LV_LOG_INFO("Exit VGLite draw thread.");
}
#endif

#endif /*LV_USE_DRAW_BES_VGLITE*/
