#ifndef __BSP_LCD_INTERNAL_H__
#define __BSP_LCD_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include "bsp_lcd_uart.h" // IWYU pragma: keep

// pixel offset
#define LCD_H_OFFSET                    26
#define LCD_V_OFFSET                    1

// pixel size
#define LCD_RGB565_PIXEL                16
#define LCD_RGB888_PIXEL                24

// lcd pixel size
#define LCD_H_RES                       (PANLE_DISPLAY_W)
#define LCD_V_RES                       (PANLE_DISPLAY_H)

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
