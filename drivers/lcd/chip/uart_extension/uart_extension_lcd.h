#ifndef __UART_EXTENSION_LCD_H__
#define __UART_EXTENSION_LCD_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
// #include <stdint.h> 

/// img id
typedef enum {
    LCD_DIPLAY_IMG_ID_NONE,
    LCD_DISPLAY_POWER_ON_LOGO = 1,   /// 开机logo
    LCD_DISPLAY_POWER_ON_QR_CODE,    /// 开机二维码
    LCD_DISPLAY_POWER_OFF_CHARGING,  /// 关机充电显示
    LCD_DISPLAY_UNLOCK_REMIND,       /// 解锁提醒
    LCD_DISPLAY_PUMP_MODE,           /// 模式显示
    LCD_DISPLAY_PUMPING_COMPLETED,   /// 吸奶完成
    LCD_DISPLAY_LEFT_PUMPING_LEVEL,  /// 左边泵奶力度展示
    LCD_DISPLAY_RIGHT_PUMPING_LEVEL, /// 右边泵奶力度展示
    LCD_DISPLAY_BREASTFEEDING_MODE, /// 追奶模式展示
    LCD_DISPLAY_HOME_PAGE, /// 主页展示
    LCD_DISPLAY_IMG_ID_MAX,
} lcd_img_id_e;


/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void knob_button_event_handler(int arg1, int arg2, void *param);
#ifdef __cplusplus
}
#endif

#endif 
