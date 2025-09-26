#ifndef __BSP_LCD_UART_H__
#define __BSP_LCD_UART_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h>
#include "bsp_common.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_lcd_uart_send_data(uint8_t *buf, uint32_t len);
int bsp_lcd_uart_init(void);
void bsp_lcd_uart_register_event_cb(panel_recv_event_cb_func cb);

#ifdef __cplusplus
}
#endif

#endif 
