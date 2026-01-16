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

/// 屏幕串口协议数据格式
#define LCD_UART_SOF                             0x5C   
#define LCD_PROTOCOL_HEADER_SIZE                 (4)
#define LCD_PROTOCOL_CHECKSUM_SIZE               (2)
#define LCD_PROTOCOL_PAYLOAD_SIZE                (256 - LCD_PROTOCOL_HEADER_SIZE - LCD_PROTOCOL_CHECKSUM_SIZE) /// 减去字节头和校验和占用空间
#define LCD_PROTOCOL_DATA_SIZE                   (LCD_PROTOCOL_PAYLOAD_SIZE - LCD_PROTOCOL_CHECKSUM_SIZE) /// 减去校验和占用空间

/// event type
typedef enum {
    LCD_UART_DISPLAY_EVENT = 1,
    LCD_UART_KNOB_BUTTON_EVENT = 2,
} lcd_uart_event_type_e;

typedef struct {
    uint8_t sof;
    uint8_t length[2];
    uint8_t srv_id;
} lcd_protocol_header_t;

typedef struct {
    lcd_protocol_header_t header;
    uint8_t payload[LCD_PROTOCOL_PAYLOAD_SIZE];
} lcd_protocol_data_t;

/// error code
typedef enum {
    LCD_RSP_SUCCESS = 0x1000,   /// 执行成功
    LCD_RSP_OP_CODE_NO_SUPPORT = 0x1001, /// 命令/操作不支持
    LCD_RSP_INVAILD_PARAMETER = 0x1002,  /// 无效/非法参数
} lcd_operational_response_code_e;

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

#ifdef __cplusplus
}
#endif

#endif 
