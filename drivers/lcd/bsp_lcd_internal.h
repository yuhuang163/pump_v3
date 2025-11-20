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
int bsp_lcd_init(void);
void lcd_backlight_control(uint8_t level);
int bsp_lcd_data_send(uint8_t srv_id, lcd_tlvs_t *tlvs, uint8_t tlvs_num);
#ifdef __cplusplus
}
#endif

#endif 
