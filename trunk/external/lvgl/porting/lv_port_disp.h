/**
 * @file lv_port_disp_templ.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

///////////////////////////////定义显示分辨率///////////////////////
#ifndef PANLE_DISPLAY_W
#define PANLE_DISPLAY_W (80)
#endif

#ifndef PANLE_DISPLAY_H
#define PANLE_DISPLAY_H (160)
#endif

#define MY_DISP_HOR_RES (PANLE_DISPLAY_W)
#define MY_DISP_VER_RES (PANLE_DISPLAY_H)
///////////////////////////////定义显示分辨率///////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/* Initialize low level display driver */
void lv_port_disp_init(void *user_data);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
