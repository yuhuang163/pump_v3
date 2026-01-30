#ifndef __BSP_LCD_INTERNAL_H__
#define __BSP_LCD_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
#include "bsp_lcd_bes_qspi.h" // IWYU pragma: keep
#else
#include "bsp_lcd_uart.h" // IWYU pragma: keep
#endif

/// 水平偏移量
#define LCD_H_OFFSET                    0
/// 垂直偏移量
#define LCD_V_OFFSET                    0

/// 像素大小
#define LCD_RGB565_PIXEL                16
#define LCD_RGB888_PIXEL                24

/// lcd 水平分辨率
#define LCD_H_RES                       (PANLE_DISPLAY_W)

/// lcd 垂直分辨率
#define LCD_V_RES                       (PANLE_DISPLAY_H)

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_lcd_init(void);
int bsp_lcd_deinit(void);
int bsp_lcd_uart_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args);
int bsp_lcd_refresh(const void *buf, uint32_t n_bytes);
#ifdef __cplusplus
}
#endif

#endif 
