#ifndef __GUIDE_UI_H__
#define __GUIDE_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

typedef enum {
    GUIDE_OPERATION_SUCCESS = 0,
    GUIDE_OPERATION_FAILED = 1,
} guide_operation_type_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ui_display_guide_prepare(ui_manager_context_t *thiz);
void ui_display_guide_remove(ui_manager_context_t *thiz);
int ui_display_guide_process(ui_manager_context_t *thiz);

void guide_tip_popup_ui_update(ui_manager_context_t *thiz, ui_proc_guide_t *proc, lv_obj_t *parent, uint8_t *ui_index);
void guide_tip_popup_ui_remove(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

