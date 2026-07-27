#ifndef __HOME_UI_H__
#define __HOME_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
/// 模式选择页数量（2 页）
#define HOME_MODE_PAGE_COUNT                                2
/// 韵律选择页：基础 2 页，第三页（个性化）由 custom 是否激活动态决定
#define HOME_RHYTHM_PAGE_COUNT_BASE                         2
#define HOME_RHYTHM_PAGE_COUNT_MAX                          3

/// 页间过渡动画参数（喂参入口）
#ifndef HOME_PAGE_TRANSITION_MS
#define HOME_PAGE_TRANSITION_MS                             280
#endif
/// 过渡时最小缩放（256 = 100%）
#ifndef HOME_PAGE_TRANSITION_SCALE_MIN
#define HOME_PAGE_TRANSITION_SCALE_MIN                      200
#endif
/// 过渡时最小透明度
#ifndef HOME_PAGE_TRANSITION_OPA_MIN
#define HOME_PAGE_TRANSITION_OPA_MIN                        102 /// ~40%
#endif
/// 邻页水平偏移（像素，相对屏幕中心）
#ifndef HOME_PAGE_TRANSITION_SIDE_OFFSET_X
#define HOME_PAGE_TRANSITION_SIDE_OFFSET_X                  120
#endif

typedef enum {
    HOME_PAGE_MAIN = 0,                                     /// 主页
    HOME_PAGE_MODE_0,                                       /// 模式选择页 0
    HOME_PAGE_MODE_1,                                       /// 模式选择页 1
    HOME_PAGE_RHYTHM_0,                                     /// 韵律选择页 0
    HOME_PAGE_RHYTHM_1,                                     /// 韵律选择页 1
    HOME_PAGE_RHYTHM_2,                                     /// 韵律选择页 2（个性化，可动态禁用）
    HOME_PAGE_MAX
} home_page_id_e;

typedef enum {
    HOME_PAGE_SLIDE_NONE = 0,
    HOME_PAGE_SLIDE_LEFT,                                   /// 内容往左移（旋钮右/下一页）
    HOME_PAGE_SLIDE_RIGHT,                                  /// 内容往右移（旋钮左/上一页）
} home_page_slide_dir_e;

#define HOME_MODE_SELECT_EFFECT_TITLE_BLINK                 0 // 仅标题闪烁
#define HOME_MODE_SELECT_EFFECT_CAPSULE                     1 // 胶囊 + 圆点指示器
#define HOME_MODE_SELECT_EFFECT_CAROUSEL                    2 // 三标题横向轮播

#ifndef HOME_MODE_SELECT_EFFECT
#define HOME_MODE_SELECT_EFFECT                             HOME_MODE_SELECT_EFFECT_CAROUSEL
#endif

#if HOME_MODE_SELECT_EFFECT == HOME_MODE_SELECT_EFFECT_TITLE_BLINK
#define HOME_MODE_TITLE_BLINK_ENABLE                        1
#define HOME_MODE_SELECT_TRANSITION_ENABLE                  0
#define HOME_MODE_SELECT_CAPSULE_ENABLE                     0
#define HOME_MODE_SELECT_CAROUSEL_ENABLE                    0
#elif HOME_MODE_SELECT_EFFECT == HOME_MODE_SELECT_EFFECT_CAPSULE
#define HOME_MODE_TITLE_BLINK_ENABLE                        0
#define HOME_MODE_SELECT_TRANSITION_ENABLE                  1
#define HOME_MODE_SELECT_CAPSULE_ENABLE                     1
#define HOME_MODE_SELECT_CAROUSEL_ENABLE                    0
#elif HOME_MODE_SELECT_EFFECT == HOME_MODE_SELECT_EFFECT_CAROUSEL
#define HOME_MODE_TITLE_BLINK_ENABLE                        0
#define HOME_MODE_SELECT_TRANSITION_ENABLE                  1
#define HOME_MODE_SELECT_CAPSULE_ENABLE                     0
#define HOME_MODE_SELECT_CAROUSEL_ENABLE                    1
#else
#error "Invalid HOME_MODE_SELECT_EFFECT"
#endif

/// 胶囊样式（仅 CAPSULE 动效下使用）
typedef enum {
    HOME_MODE_CAPSULE_STYLE_NONE = 0,
    HOME_MODE_CAPSULE_STYLE_COMMON,    
    HOME_MODE_CAPSULE_STYLE_BF,        
} home_mode_capsule_style_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 开机LOGO页面
void ui_display_home_prepare(ui_manager_context_t *thiz);
void ui_display_home_remove(ui_manager_context_t *thiz);
int ui_display_home_process(ui_manager_context_t *thiz);
/// 旋钮切页入口（左/右滑动过渡）
void ui_display_home_page_switch(home_page_slide_dir_e dir);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__
