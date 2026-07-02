#ifndef __HOME_NIGHT_LIGHT_ADJUST_UI_H__
#define __HOME_NIGHT_LIGHT_ADJUST_UI_H__

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

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

/// 夜灯调节界面
void ui_display_home_night_light_adjust_prepare(ui_manager_context_t *thiz);
void ui_display_home_night_light_adjust_remove(ui_manager_context_t *thiz);
int ui_display_home_night_light_adjust_process(ui_manager_context_t *thiz);
   
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

