#ifndef __PUMP_HOME_UI_H__
#define __PUMP_HOME_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep
#include "battery_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef enum {
    PUMP_HOME_PAGE_MAIN = 0,                                /// 主页
    PUMP_HOME_PAGE_MODE_0,                                  /// 模式选择页 0
    PUMP_HOME_PAGE_MODE_1,                                  /// 模式选择页 1
    PUMP_HOME_PAGE_RHYTHM_0,                                /// 韵律选择页 0
    PUMP_HOME_PAGE_RHYTHM_1,                                /// 韵律选择页 1
    PUMP_HOME_PAGE_RHYTHM_2,                                /// 韵律选择页 2（个性化，可动态禁用）
    PUMP_HOME_PAGE_MAX
} pump_home_page_id_e;

typedef enum {
    PUMP_HOME_PAGE_SLIDE_NONE = 0,
    PUMP_HOME_PAGE_SLIDE_LEFT,                              /// 内容往左移（旋钮右/下一页）
    PUMP_HOME_PAGE_SLIDE_RIGHT,                             /// 内容往右移（旋钮左/上一页）
} pump_home_page_slide_dir_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 泵奶主界面
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz);
void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz);
int ui_display_pump_home_screen_process(ui_manager_context_t *thiz);
/// 旋钮切页入口（左/右滑动过渡）
void ui_display_pump_home_page_switch(pump_home_page_slide_dir_e dir);

#ifdef __cplusplus
}
#endif
#endif // __PUMP_HOME_UI_H__
