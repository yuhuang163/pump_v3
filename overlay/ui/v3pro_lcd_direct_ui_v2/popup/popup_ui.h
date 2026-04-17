#ifndef __POPUP_UI_H__
#define __POPUP_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void popup_window_ui_prepare(ui_manager_context_t *thiz, popup_ui_id_e type, uint16_t time);
void popup_window_ui_remove(ui_manager_context_t *thiz);
int popup_window_ui_process(ui_manager_context_t *thiz, popup_ui_id_e type);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

