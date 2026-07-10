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
#define PUMP_HOME_MODE_SELECT_EFFECT_TITLE_BLINK            0 // 仅标题闪烁
#define PUMP_HOME_MODE_SELECT_EFFECT_CAPSULE                1 // 胶囊 + 圆点指示器
#define PUMP_HOME_MODE_SELECT_EFFECT_CAROUSEL               2 // 三标题横向轮播

#ifndef PUMP_HOME_MODE_SELECT_EFFECT
#define PUMP_HOME_MODE_SELECT_EFFECT                        PUMP_HOME_MODE_SELECT_EFFECT_CAROUSEL
#endif

#if PUMP_HOME_MODE_SELECT_EFFECT == PUMP_HOME_MODE_SELECT_EFFECT_TITLE_BLINK
#define PUMP_HOME_MODE_TITLE_BLINK_ENABLE                   1
#define PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE             0
#define PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE                0
#define PUMP_HOME_MODE_SELECT_CAROUSEL_ENABLE               0
#elif PUMP_HOME_MODE_SELECT_EFFECT == PUMP_HOME_MODE_SELECT_EFFECT_CAPSULE
#define PUMP_HOME_MODE_TITLE_BLINK_ENABLE                   0
#define PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE             1
#define PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE                1
#define PUMP_HOME_MODE_SELECT_CAROUSEL_ENABLE               0
#elif PUMP_HOME_MODE_SELECT_EFFECT == PUMP_HOME_MODE_SELECT_EFFECT_CAROUSEL
#define PUMP_HOME_MODE_TITLE_BLINK_ENABLE                   0
#define PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE             1
#define PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE                0
#define PUMP_HOME_MODE_SELECT_CAROUSEL_ENABLE               1
#else
#error "Invalid PUMP_HOME_MODE_SELECT_EFFECT"
#endif

/// 胶囊样式（仅 CAPSULE 动效下使用）
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
