/**
 * @file lv_vglite_utils.h
 *
 */

/**
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_VGLITE_UTILS_H
#define LV_VGLITE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_vglite.h"

#if LV_USE_DRAW_BES_VGLITE

/*********************
 *      DEFINES
 *********************/

#define ENUM_TO_STRING(e) \
    case (e):             \
    return #e

#if LV_USE_BES_VGLITE_ASSERT
#define VGLITE_ASSERT(expr) LV_ASSERT(expr)
#else
#define VGLITE_ASSERT(expr)
#endif

#define VGLITE_ASSERT_MSG(expr, msg)                                 \
    do {                                                             \
        if(!(expr)) {                                                \
            LV_LOG_ERROR(msg);                                       \
            VGLITE_ASSERT(false);                                    \
        }                                                            \
    } while(0)

#define VGLITE_CHECK_ERROR(function)                                 \
    do {                                                             \
        vg_lite_error_t error = function;                            \
        if(error != VG_LITE_SUCCESS) {                               \
            LV_LOG_ERROR("Execute '" #function "' error(%d): %s",    \
                         (int)error, vglite_error_to_string(error)); \
            VGLITE_ASSERT(false);                                    \
        }                                                            \
    } while (0)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**
 * Set the clipping box.
 *
 * @param[in] clip_area Clip area with relative coordinates of destination buffer
 *
 */
static inline void vglite_set_scissor(const lv_area_t * clip_area);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

const char * vglite_error_to_string(vg_lite_error_t error);

/**
 * Flush command to VG-Lite.
 *
 */
void vglite_run(void);

/**
 * Get vglite color. Premultiplies (if not hw already) and swizzles the given
 * LVGL 32bit color to obtain vglite color.
 *
 * @param[in] lv_col32 The initial LVGL 32bit color
 * @param[in] gradient True for gradient color
 *
 * @retval The vglite 32-bit color value:
 *
 */
vg_lite_color_t vglite_get_color(lv_color32_t lv_col32, bool gradient);

/**
 * Get vglite blend mode.
 *
 * @param[in] lv_blend_mode The LVGL blend mode
 *
 * @retval The vglite blend mode
 *
 */
vg_lite_blend_t vglite_get_blend_mode(lv_blend_mode_t lv_blend_mode);

/**
 * Get vglite buffer format.
 *
 * @param[in] cf Color format
 *
 * @retval The vglite buffer format
 *
 */
vg_lite_buffer_format_t vglite_get_buf_format(lv_color_format_t cf);

/**
 * Get vglite buffer stride
 *
 * @param[in] cf Color format
 * @param[in] w buffer width
 *
 * @retval The vglite buffer stride
 *
 */
uint32_t vglite_get_buf_stride(lv_color_format_t cf, uint32_t w);

/**
 * Check memory and stride alignment.
 *
 * @param[in] buf Buffer address
 * @param[in] width width of buffer in pixels
 * @param[in] stride Stride of buffer in bytes
 * @param[in] cf Color format - to calculate the expected alignment
 *
 * @retval true Alignment OK
 *
 */
bool vglite_buf_aligned(const void * buf, uint32_t width, uint32_t stride, lv_color_format_t cf);

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline void vglite_set_scissor(const lv_area_t * clip_area)
{
    vg_lite_set_scissor(clip_area->x1, clip_area->y1, clip_area->x2 + 1, clip_area->y2 + 1);
    vg_lite_enable_scissor();
}

#endif /*LV_USE_DRAW_BES_VGLITE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_VGLITE_UTILS_H*/
