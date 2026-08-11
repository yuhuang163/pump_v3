#ifndef __POWER_ON_HOLD_UI_H__
#define __POWER_ON_HOLD_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

#if IS_SUPPORT_POWER_ON_GUIDE_UI

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ui_display_power_on_hold_prepare(ui_manager_context_t *thiz);
void ui_display_power_on_hold_remove(ui_manager_context_t *thiz);
int ui_display_power_on_hold_process(ui_manager_context_t *thiz);

#endif /* IS_SUPPORT_POWER_ON_GUIDE_UI */

#ifdef __cplusplus
}
#endif
#endif // __POWER_ON_HOLD_UI_H__
