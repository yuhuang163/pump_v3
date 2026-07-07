/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_popup_disp_info_t *operation_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool ui_popup_is_supported(popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
        case ID_POPUP_UI_PUMP_WILL_FINISH_TIP:
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
        case ID_POPUP_UI_PUMP_PAUSE_SETTLE:
        case ID_POPUP_UI_PUMP_PAUSE_MANUAL:
        case ID_POPUP_UI_PUMP_PAUSE_TIMEOUT:
        case ID_POPUP_UI_PUMP_RUNING_30M_TIP:
        case ID_POPUP_UI_OP_SUCCESS:
        case ID_POPUP_UI_OP_FAILED:
        case ID_POPUP_UI_AIRPLANE_MODE_PROMPT:
        case ID_POPUP_UI_FACTORY_RESET_CONFIRM:
        case ID_POPUP_UI_THEME_RESELECT:
            return true;
        default:
            return false;
    }
}

static void ui_popup_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    EXCEPTION_CHECK_VOID(info);
    if (info->runtime.force_update) {
        ui_popup_components_hide_all();
    }

    switch (popup_type) {
        case ID_POPUP_UI_PUMP_PAUSE_MANUAL:
        case ID_POPUP_UI_PUMP_PAUSE_TIMEOUT:
        case ID_POPUP_UI_PUMP_RUNING_30M_TIP:
            ui_popup_pump_pause_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
        case ID_POPUP_UI_PUMP_WILL_FINISH_TIP:
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
        case ID_POPUP_UI_PUMP_PAUSE_SETTLE:
            ui_popup_pump_finish_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_OP_SUCCESS:
        case ID_POPUP_UI_OP_FAILED:
            ui_popup_operation_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_AIRPLANE_MODE_PROMPT:
            ui_popup_airplane_mode_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_FACTORY_RESET_CONFIRM:
            ui_popup_factory_reset_confirm_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_THEME_RESELECT:
            ui_popup_theme_ui_update(thiz, popup_type);
            break;
        default:
            break;
    }
}

static void ui_popup_prepare(ui_manager_context_t *thiz, lv_obj_t *parent, popup_ui_id_e type)
{
    uint16_t time = 0;

    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (operation_info == NULL) {
        operation_info = (ui_popup_disp_info_t *)u_malloz(sizeof(ui_popup_disp_info_t));
        EXCEPTION_CHECK_VOID(operation_info);
    }
    ui_popup_root_init(thiz, parent);
    operation_info->arc.ring_progress = 0;
    time = ui_popup_target_time_get(type);
    operation_info->runtime.target_time = time;
    operation_info->countdown.max_value = time / 1000;
    operation_info->arc.ring_progress_max = (time > 0) ?
        ((time / UI_POPUP_STATUS_DISPLAY_STEP_TIME) - UI_POPUP_STATUS_ARC_RING_PROGRESS_LEAD_STEPS) : 0;
    operation_info->runtime.force_update = true;
    operation_info->runtime.start_time = get_timestamp();
    LOG_UI("pump status popup prepare %s", ui_popup_name_get(thiz, type));
}

static void ui_popup_process(ui_manager_context_t *thiz, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (operation_info == NULL || operation_info->root.obj == NULL ||
        !lv_obj_is_valid(operation_info->root.obj)) {
        return;
    }
    ui_popup_ui_update(thiz, type);
    operation_info->runtime.force_update = false;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
ui_popup_disp_info_t *ui_popup_info_get(void)
{
    return operation_info;
}

void ui_popup_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (operation_info == NULL) {
        return;
    }
    for (uint8_t i = 0; i < UI_POPUP_ICON_COUNT; i++) {
        ui_img_baj_release_decoded_buf(operation_info->icon.obj[i]);
    }
    LVGL_OBJ_DELETE(operation_info->root.obj);
    u_free(operation_info);
    operation_info = NULL;
}

void ui_popup_update(ui_manager_context_t *thiz,
    popup_ui_id_e type, lv_obj_t *parent, uint8_t *ui_index)
{
    if (type == ID_POPUP_UI_NONE) {
        ui_popup_remove(thiz);
        *ui_index = ID_POPUP_UI_NONE;
        return;
    }

    if (!ui_popup_is_supported(type)) {
        return;
    }

    if (*ui_index != type) {
        ui_popup_remove(thiz);
        ui_popup_prepare(thiz, parent, type);
        *ui_index = type;
    } else {
        ui_popup_process(thiz, type);
    }
}
