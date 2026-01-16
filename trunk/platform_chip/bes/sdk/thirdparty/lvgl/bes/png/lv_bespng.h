/**
 * @file lv_bespng.h
 *
 */

#ifndef LV_BESPNG_H
#define LV_BESPNG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../src/lv_conf_internal.h"
#if LV_USE_BESPNG

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
void lv_bespng_init(void);

void lv_bespng_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_BESPNG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_BESPNG_H*/
