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

#ifndef PUMP_HOME_MODE_TITLE_BLINK_ENABLE
#define PUMP_HOME_MODE_TITLE_BLINK_ENABLE                 (0)
#endif

#ifndef PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE
#define PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE           (1)
#endif

typedef enum {
    PUMP_HOME_CAPSULE_STYLE_NONE = 0,
    PUMP_HOME_CAPSULE_STYLE_COMMON,
    PUMP_HOME_CAPSULE_STYLE_BF,
} pump_home_capsule_style_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 泵奶主界面
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz);
void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz);
int ui_display_pump_home_screen_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif // __PUMP_HOME_UI_H__

