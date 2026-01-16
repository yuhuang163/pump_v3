/**
 * @file lv_vglite_freetype.h
 *
 */

/**
 * Copyright 2025 BES
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_VGLITE_FREETYPE_H
#define LV_VGLITE_FREETYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_vglite.h"

#if LV_USE_DRAW_BES_VGLITE && LV_USE_FREETYPE

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
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize freetype outline callbacks.
 */
void vglite_freetype_init_outline_cb(void);

/**
 * Draw with current outline path data.
 */
int vglite_freetype_draw(lv_draw_task_t * t, lv_draw_glyph_dsc_t * glyph_draw_dsc);


/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* LV_USE_DRAW_BES_VGLITE && LV_USE_FREETYPE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_VGLITE_FREETYPE_H */
