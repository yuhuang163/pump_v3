/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_lcd_uart.h"
#include "bsp_lcd_internal.h"
#include "uart_extension_lcd.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
__WEAK void knob_button_event_handler(int arg1, int arg2, void *param)
{

}

void lcd_backlight_control(uint8_t level)
{
    lcd_tlvs_t *tlvs = u_malloz(sizeof(lcd_tlvs_t) * 1);
    tlvs[0].type = LCD_BACKLIGHT_LEVEL;
    tlvs[0].data_size = 1;
    tlvs[0].data[0] = level > 100 ? 100 : level;
    bsp_lcd_data_send(LCD_UART_DISPLAY_EVENT, tlvs, 1);
    // LOG_PBUS("lcd backlight control: %d", level);
    u_free(tlvs);
}

int bsp_lcd_init(void) 
{
    bsp_lcd_uart_register_event_cb(LCD_UART_KNOB_BUTTON_EVENT, knob_button_event_handler);
    return bsp_lcd_uart_init();
}


