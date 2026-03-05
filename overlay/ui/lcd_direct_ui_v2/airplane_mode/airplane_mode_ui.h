#ifndef __AIRPLANE_MODE_UI_H__
#define __AIRPLANE_MODE_UI_H__

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
void ui_display_airplane_mode_prepare(ui_manager_context_t *thiz);
void ui_display_airplane_mode_remove(ui_manager_context_t *thiz);
int ui_display_airplane_mode_process(ui_manager_context_t *thiz);

void ui_display_airplane_mode_exit_init(ui_manager_context_t *thiz, lv_obj_t *root);
void ui_display_airplane_mode_exit_deinit(ui_manager_context_t *thiz, lv_obj_t *root);
void ui_display_airplane_mode_exit_process(ui_manager_context_t *thiz, lv_obj_t *root, uint8_t need_show_countdown_time);

#ifdef __cplusplus
}
#endif
#endif// __DUMMY_UI_H__

