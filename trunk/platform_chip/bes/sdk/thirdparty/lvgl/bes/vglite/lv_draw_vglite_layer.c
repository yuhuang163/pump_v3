/**
 * @file lv_draw_vglite_layer.c
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
#include "src/core/lv_global.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

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

void lv_draw_vglite_layer(lv_draw_task_t * t, const lv_draw_image_dsc_t * draw_dsc,
                          const lv_area_t * coords)
{
    lv_layer_t * layer_to_draw = (lv_layer_t *)draw_dsc->src;
    const lv_draw_buf_t * draw_buf = layer_to_draw->draw_buf;

    /* It can happen that nothing was draw on a layer and therefore its buffer is not allocated.
     * In this case just return.
     */
    if(draw_buf == NULL)
        return;

    LV_PROFILER_DRAW_BEGIN;

    if(_draw_info.unit_cnt > 1) {
        lv_draw_vglite_sync_invalidate(draw_buf->data, draw_buf->data_size);
    }

    lv_draw_image_dsc_t new_draw_dsc = *draw_dsc;
    new_draw_dsc.src = draw_buf;
    lv_draw_vglite_img(t, &new_draw_dsc, coords);

    LV_PROFILER_DRAW_END;
}

#endif /*LV_USE_DRAW_BES_VGLITE*/
