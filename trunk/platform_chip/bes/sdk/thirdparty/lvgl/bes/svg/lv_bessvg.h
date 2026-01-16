/**
 * @file lv_bessvg.h
 *
 */

#ifndef LV_BESSVG_H
#define LV_BESSVG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../src/lv_conf_internal.h"
#if LV_USE_BESSVG

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Register the PNG decoder functions in LVGL
 */
void lv_bessvg_init(void);

void lv_bessvg_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_BESSVG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_BESSVG_H*/
