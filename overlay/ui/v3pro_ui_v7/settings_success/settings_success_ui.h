#ifndef __SETTINGS_SUCCESS_UI_H__
#define __SETTINGS_SUCCESS_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
/** 默认成功文案（Success） */
#define SETTINGS_SUCCESS_STYLE_DEFAULT              (0)
/** 韵律切换确认后成功文案（Program switched / Timer reset） */
#define SETTINGS_SUCCESS_STYLE_PROGRAM_SWITCHED     (1)

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ui_display_settings_success_prepare(ui_manager_context_t *thiz, uint8_t style);
void ui_display_settings_success_remove(ui_manager_context_t *thiz);
int ui_display_settings_success_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif // __SETTINGS_SUCCESS_UI_H__
