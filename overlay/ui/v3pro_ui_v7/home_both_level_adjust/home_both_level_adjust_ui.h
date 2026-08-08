#ifndef __HOME_BOTH_LEVEL_ADJUST_UI_H__
#define __HOME_BOTH_LEVEL_ADJUST_UI_H__

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
uint32_t home_both_level_adjust_over_limit_color_get(uint8_t pump_mode);
void ui_display_home_both_level_adjust_prepare(ui_manager_context_t *thiz);
void ui_display_home_both_level_adjust_remove(ui_manager_context_t *thiz);
int ui_display_home_both_level_adjust_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif // __HOME_BOTH_LEVEL_ADJUST_UI_H__
