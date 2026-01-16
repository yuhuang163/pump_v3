/**
 * @file lv_besetc2.h
 *
 */

#ifndef LV_BESETC2_H
#define LV_BESETC2_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../src/lv_conf_internal.h"
#if LV_USE_BESETC2

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
void lv_besetc2_init(void);

void lv_besetc2_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_BESETC2*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_BESETC2_H*/
