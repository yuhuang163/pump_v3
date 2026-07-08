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
#ifndef HOME_MODE_TITLE_BLINK_ENABLE
#define HOME_MODE_TITLE_BLINK_ENABLE                      (0)
#endif

#ifndef HOME_MODE_SELECT_TRANSITION_ENABLE
#define HOME_MODE_SELECT_TRANSITION_ENABLE                (1)
#endif

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

