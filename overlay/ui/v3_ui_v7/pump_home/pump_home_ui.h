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
/// 模式选择页数量（2 页）
#define PUMP_HOME_MODE_PAGE_COUNT                           2
/// 韵律选择页：基础 2 页，第三页（个性化）由 custom 是否激活动态决定
#define PUMP_HOME_RHYTHM_PAGE_COUNT_BASE                    2
#define PUMP_HOME_RHYTHM_PAGE_COUNT_MAX                     3

/// 页间过渡动画参数（喂参入口）
#ifndef PUMP_HOME_PAGE_TRANSITION_MS
#define PUMP_HOME_PAGE_TRANSITION_MS                        280
#endif
/// 过渡时最小缩放（256 = 100%）
#ifndef PUMP_HOME_PAGE_TRANSITION_SCALE_MIN
#define PUMP_HOME_PAGE_TRANSITION_SCALE_MIN                 200
#endif
/// 过渡时最小透明度
#ifndef PUMP_HOME_PAGE_TRANSITION_OPA_MIN
#define PUMP_HOME_PAGE_TRANSITION_OPA_MIN                   102 /// ~40%
#endif
/// 邻页水平偏移（像素，相对屏幕中心）
#ifndef PUMP_HOME_PAGE_TRANSITION_SIDE_OFFSET_X
#define PUMP_HOME_PAGE_TRANSITION_SIDE_OFFSET_X             120
#endif

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
/// 旋钮切页入口（左/右滑动过渡）
void ui_display_pump_home_page_switch(pump_home_page_slide_dir_e dir);

#ifdef __cplusplus
}
#endif
#endif // __PUMP_HOME_UI_H__
