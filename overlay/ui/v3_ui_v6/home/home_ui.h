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
   
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__
