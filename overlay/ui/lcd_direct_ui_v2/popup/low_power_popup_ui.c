/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_popup.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BATTERY_LOW_POWER_POP_TEXT_COLOR                                    0xFFFFFF
#define BATTERY_LOW_POWER_POP_ARC_SIZE                                      (360)    // 圆弧尺寸
#define BATTERY_LOW_POWER_POP_ARC_WIDTH                                     (12)      // 圆弧粗细
#define BATTERY_LOW_POWER_POP_ARC_COLOR                                     (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define BATTERY_LOW_POWER_POP_ARC_START_ANGLE                               (270)    
#define BATTERY_LOW_POWER_POP_ARC_FULL_SWEEP                                (360)    
#define BATTERY_LOW_POWER_POP_DISPLAY_TIME                                  (3000)  
#define BATTERY_LOW_POWER_POP_DISPLAY_STEP_TIME                             (100)
#define BATTERY_LOW_POWER_POP_DISPLAY_MAX_VALUE                             ((BATTERY_LOW_POWER_POP_DISPLAY_TIME / \
    BATTERY_LOW_POWER_POP_DISPLAY_STEP_TIME) - 2)
  
typedef struct {    
    lv_obj_t * root;
    lv_obj_t * text;
    lv_obj_t * arc;                   /// 圆弧进度条对象
    lv_obj_t * number_text;           /// 数字文本对象
    lv_obj_t * mask_layer;            /// 遮罩层对象
    uint8_t force_update;
    uint8_t display_number;           /// 显示倒计时数字
    uint16_t ring_progress;           /// 圆弧进度
    uint32_t start_time;              /// 倒计时起始时间
} battery_low_power_display_info_t;

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
void ui_display_low_power_popup_prepare(ui_manager_context_t *thiz)
{

}

void ui_display_low_power_popup_remove(ui_manager_context_t *thiz)
{

}

int ui_display_low_power_popup_process(ui_manager_context_t *thiz)
{
    return 0;
}