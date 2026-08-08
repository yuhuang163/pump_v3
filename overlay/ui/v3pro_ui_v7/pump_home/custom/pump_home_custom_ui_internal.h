#ifndef __PUMP_HOME_CUSTOM_UI_INTERNAL_H__
#define __PUMP_HOME_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "pump_home_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_custom_ui_prepare(ui_manager_context_t *thiz);
void pump_home_custom_ui_remove(ui_manager_context_t *thiz);
int pump_home_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_HOME_CUSTOM_UI_INTERNAL_H__
