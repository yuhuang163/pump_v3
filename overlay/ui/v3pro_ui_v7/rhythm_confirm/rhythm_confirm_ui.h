#ifndef __RHYTHM_CONFIRM_UI_H__
#define __RHYTHM_CONFIRM_UI_H__

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
void ui_display_rhythm_confirm_prepare(ui_manager_context_t *thiz);
void ui_display_rhythm_confirm_remove(ui_manager_context_t *thiz);
int ui_display_rhythm_confirm_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif // __RHYTHM_CONFIRM_UI_H__
