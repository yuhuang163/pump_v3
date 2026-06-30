#ifndef __SYSTEM_ABNORMAL_UI_H__
#define __SYSTEM_ABNORMAL_UI_H__

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
/// 系统异常页面
void ui_display_system_abnormal_prepare(ui_manager_context_t *thiz);
void ui_display_system_abnormal_remove(ui_manager_context_t *thiz);
int ui_display_system_abnormal_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __SYSTEM_ABNORMAL_UI_H__
