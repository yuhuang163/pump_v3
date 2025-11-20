#ifndef __UI_IMG_DEFINE_H__
#define __UI_IMG_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/// 图片ID类型
typedef enum {
    UI_IMG_ID_LOGO = 1,  /// 开机LOGO
    UI_IMG_ID_BOOT_QRCODE = 2,   /// 开机二维码
    UI_IMG_ID_CHARGE = 3,   /// 充电中
    UI_IMG_ID_UNLOCK_TIP = 4,  /// 解锁提示
    UI_ID_PUMP_MODE_TIP = 5,     /// 泵模式提示
    UI_IMG_ID_PUMP_COMPLETE_TIP = 6, /// 吸奶完成提示
    UI_IMG_ID_LEFT_LEVEL_ADJUST_DISPLAY = 7,    /// 左档位调节显示
    UI_IMG_ID_RIGHT_LEVEL_ADJUST_DISPLAY = 8,   /// 右档位调节显示
    UI_IMG_ID_SETTINGS_SUBMENU = 9,   /// 追奶模式提示
    UI_IMG_ID_HOME_SCREEN = 0x0A,   /// 首页
    UI_IMG_ID_PUMP_SPEED = 0x0B,    /// 泵速度显示
    UI_IMG_ID_PUMP_CHARGER_NOT_MATCH = 0x0C,    /// 泵电源适配器不匹配
    UI_IMG_ID_PUMP_BIND_SUCCESS = 0x0D,    /// 泵绑定成功
    UI_IMG_ID_FACTORY_QRCODE = 0x0E,    /// 工厂二维码
    
    UI_IMG_ID_CHASING_MILK_MODE_GUIDE = 0x10,    /// 追奶模式切换引导
    UI_IMG_ID_PUMP_LEVEL_ADJUST_GUIDE = 0x11,    /// 泵奶力度调节引导
    UI_IMG_ID_APP_MILK_RECORD_GUIDE = 0x12,    /// APP奶量记录引导
    UI_IMG_ID_PRODUCT_TESTS_RESOUCE = 0x13,    /// 产测主界面资源显示

} ui_img_id_type_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


