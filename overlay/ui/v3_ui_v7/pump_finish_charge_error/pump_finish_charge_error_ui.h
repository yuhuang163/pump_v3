#ifndef __PUMP_FINISH_CHARGE_ERROR_UI_H__
#define __PUMP_FINISH_CHARGE_ERROR_UI_H__

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
void ui_display_pump_finish_charge_error_prepare(ui_manager_context_t *thiz);
void ui_display_pump_finish_charge_error_remove(ui_manager_context_t *thiz);
int ui_display_pump_finish_charge_error_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif // __PUMP_FINISH_CHARGE_ERROR_UI_H__
