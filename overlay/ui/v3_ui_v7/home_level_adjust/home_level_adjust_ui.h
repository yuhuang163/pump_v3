#ifndef __HOME_LEVEL_ADJUST_UI_H__
#define __HOME_LEVEL_ADJUST_UI_H__

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
/** 按子模式（pump_mode_type_e）取力度上限：刺激 0-5，其它 0-15 */
uint8_t home_level_adjust_max_get(uint8_t mode_type);
/** 常规模式用 pump_mode；韵律模式用 runtime 子模式 */
uint8_t home_level_adjust_mode_type_resolve(uint8_t pump_mode, uint8_t runtime_mode_type);
const char *home_level_adjust_mode_type_name_get(uint8_t mode_type);
uint32_t home_level_adjust_accent_color_get(uint8_t pump_mode);
uint8_t home_level_adjust_ui_level_max_get(void);
void ui_display_home_level_adjust_prepare(ui_manager_context_t *thiz);
void ui_display_home_level_adjust_remove(ui_manager_context_t *thiz);
int ui_display_home_level_adjust_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif // __HOME_LEVEL_ADJUST_UI_H__
