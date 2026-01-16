/**
 * @file lv_draw_vglite.h
 *
 */

/**
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_DRAW_VGLITE_H
#define LV_DRAW_VGLITE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "src/lvgl.h"

#if LV_USE_DRAW_BES_VGLITE
#include "src/draw/lv_draw_private.h"
#include "src/misc/lv_area_private.h"
#include "src/draw/lv_draw_triangle.h"
#include "vg_lite.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_cache.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_draw_unit_t base_unit;
    lv_draw_task_t * task_act;
#if LV_USE_BES_VGLITE_DRAW_THREAD
    lv_thread_sync_t sync;
    lv_thread_t thread;
    volatile bool inited;
    volatile bool exit_status;
#endif
    uint32_t idx;
} lv_draw_vglite_unit_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_buf_vglite_init_handlers(void);

void lv_draw_vglite_init(void);

void lv_draw_vglite_deinit(void);

void lv_draw_vglite_arc(lv_draw_task_t * t, const lv_draw_arc_dsc_t * dsc,
                        const lv_area_t * coords);

void lv_draw_vglite_border(lv_draw_task_t * t, const lv_draw_border_dsc_t * dsc,
                           const lv_area_t * coords);

void lv_draw_vglite_box_shadow(lv_draw_task_t * t, const lv_draw_box_shadow_dsc_t * dsc,
                        const lv_area_t * coords);

void lv_draw_vglite_fill(lv_draw_task_t * t, const lv_draw_fill_dsc_t * dsc,
                         const lv_area_t * coords);

void lv_draw_vglite_img(lv_draw_task_t * t, const lv_draw_image_dsc_t * dsc,
                        const lv_area_t * coords);

void lv_draw_vglite_letter(lv_draw_task_t * t, const lv_draw_letter_dsc_t * dsc,
                          const lv_area_t * coords);

void lv_draw_vglite_label(lv_draw_task_t * t, const lv_draw_label_dsc_t * dsc,
                          const lv_area_t * coords);

void lv_draw_vglite_layer(lv_draw_task_t * t, const lv_draw_image_dsc_t * draw_dsc,
                          const lv_area_t * coords);

void lv_draw_vglite_line(lv_draw_task_t * t, const lv_draw_line_dsc_t * dsc);

void lv_draw_vglite_triangle(lv_draw_task_t * t, const lv_draw_triangle_dsc_t * dsc);

void lv_draw_vglite_mask_rect(lv_draw_task_t * t, const lv_draw_mask_rect_dsc_t * dsc,
                               const lv_area_t * coords);

#if LV_USE_VECTOR_GRAPHIC
void lv_draw_vglite_vector(lv_draw_task_t * t, const lv_draw_vector_dsc_t * dsc);
#endif

void lv_draw_vglite_sync_invalidate(const void *data, uint32_t size);

/**********************
 *      MACROS
 **********************/
#endif /*LV_USE_DRAW_BES_VGLITE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_VGLITE_H*/
