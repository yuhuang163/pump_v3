#ifndef __FACTORY_RESET_UI_H__
#define __FACTORY_RESET_UI_H__

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
void ui_display_factory_reset_prepare(ui_manager_context_t *thiz);
void ui_display_factory_reset_remove(ui_manager_context_t *thiz);
int ui_display_factory_reset_process(ui_manager_context_t *thiz);

void ui_display_power_off_prepare(ui_manager_context_t *thiz);
void ui_display_power_off_remove(ui_manager_context_t *thiz);
int ui_display_power_off_process(ui_manager_context_t *thiz);

void ui_display_will_power_off_prepare(ui_manager_context_t *thiz);
void ui_display_will_power_off_remove(ui_manager_context_t *thiz);
int ui_display_will_power_off_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

