#ifndef __LOGO_UI_H__
#define __LOGO_UI_H__

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
/// 开机LOGO icon 页面
void ui_display_logo_prepare(ui_manager_context_t *thiz);
void ui_display_logo_remove(ui_manager_context_t *thiz);
int ui_display_logo_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __LOGO_UI_H__
