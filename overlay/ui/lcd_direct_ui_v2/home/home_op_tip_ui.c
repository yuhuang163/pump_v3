/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "common_ui.h" // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_tip_popup_ui_update(ui_manager_context_t *thiz, ui_proc_home_t *proc, lv_obj_t *parent, uint8_t *ui_index)
{
    switch (proc->popup_ui_type) {
        case HOME_POPUP_UI_NONE:
            ui_common_operation_tip_remove(thiz);
            *ui_index = HOME_POPUP_UI_NONE;
            break;
        case HOME_POPUP_UI_OPERATION_SUCCESS:
            if (*ui_index != HOME_POPUP_UI_OPERATION_SUCCESS) {
                ui_common_operation_tip_remove(thiz);
                ui_common_operation_tip_prepare(thiz, parent);
                *ui_index = HOME_POPUP_UI_OPERATION_SUCCESS;
            } else {
                ui_common_operation_tip_process(thiz, COMMON_OPERATION_SUCCESS, proc->popup_ui_start_time);
            }
            break;
        case HOME_POPUP_UI_OPERATION_FAILED:
            if (*ui_index != HOME_POPUP_UI_OPERATION_FAILED) {
                ui_common_operation_tip_remove(thiz);
                ui_common_operation_tip_prepare(thiz, parent);
                *ui_index = HOME_POPUP_UI_OPERATION_FAILED;
            } else {
                ui_common_operation_tip_process(thiz, COMMON_OPERATION_FAILED, proc->popup_ui_start_time);
            }
            break;
    }
}

void home_tip_popup_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    ui_common_operation_tip_remove(thiz);
}