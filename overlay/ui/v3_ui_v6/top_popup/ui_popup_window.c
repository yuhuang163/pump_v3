/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static popup_window_disp_info_t *popup_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool popup_window_is_supported(popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_BT_CONNECT:
        case ID_POPUP_UI_BT_DISCONNECT:
        case ID_POPUP_UI_BT_CONNECTING:
        case ID_POPUP_UI_DEVICE_BIND:
        case ID_POPUP_UI_DEVICE_UNBIND:
        case ID_POPUP_UI_COUNT_DOWN:
        case ID_POPUP_UI_LOW_POWER:
        case ID_POPUP_UI_WILL_POWER_OFF:
        case ID_POPUP_UI_SYS_ERROR:
        case ID_POPUP_UI_TEMP_ERROR:
            return true;
        default:
            return false;
    }
}

static void popup_window_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    EXCEPTION_CHECK_VOID(info);
    if (!popup_window_is_supported(popup_type)) {
        return;
    }
    if (info->runtime.force_update) {
        popup_window_components_hide_all();
    }

    switch (popup_type) {
        case ID_POPUP_UI_BT_CONNECT:
        case ID_POPUP_UI_BT_DISCONNECT:
        case ID_POPUP_UI_BT_CONNECTING:
            popup_window_ble_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_DEVICE_BIND:
        case ID_POPUP_UI_DEVICE_UNBIND:
            popup_window_device_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_COUNT_DOWN:
        case ID_POPUP_UI_LOW_POWER:
        case ID_POPUP_UI_WILL_POWER_OFF:
            popup_window_power_ui_update(thiz, popup_type);
            break;
        case ID_POPUP_UI_SYS_ERROR:
        case ID_POPUP_UI_TEMP_ERROR:
            popup_window_error_ui_update(thiz, popup_type);
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
popup_window_disp_info_t *popup_window_info_get(void)
{
    return popup_info;
}

void popup_window_ui_prepare(ui_manager_context_t *thiz, popup_ui_id_e type, uint16_t time)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (popup_info == NULL) {
        popup_info = (popup_window_disp_info_t *)u_malloz(sizeof(popup_window_disp_info_t));
        EXCEPTION_CHECK_VOID(popup_info);
    }
    popup_window_root_init(thiz);
    popup_info->arc.ring_progress = 0;
    popup_info->runtime.target_time = time;
    popup_info->countdown.max_value = time / 1000;
    popup_info->arc.ring_progress_max = (time > 0) ?
        ((time / POPUP_WINDOW_STATUS_DISPLAY_STEP_TIME) - 2) : 0;
    popup_info->runtime.force_update = true;
    popup_info->runtime.start_time = get_timestamp();
    LOG_UI("popup: %s, time:%d", popup_window_name_get(thiz, type), time);
}

void popup_window_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (popup_info == NULL) {
        return;
    }
    ui_img_baj_release_decoded_buf(popup_info->icon.obj);
#if IS_SUPPORT_POPUP_WINDOW_COMP_BG
    ui_img_png_release_decoded_buf(popup_info->root.comp_bg);
#endif
    LVGL_OBJ_DELETE(popup_info->root.obj);
    u_free(popup_info);
    popup_info = NULL;
}

int popup_window_ui_process(ui_manager_context_t *thiz, popup_ui_id_e type)
{
    EXCEPTION_CHECK_INT(thiz && popup_info && popup_info->root.obj);
    popup_window_ui_update(thiz, type);
#if IS_SUPPORT_POPUP_WINDOW_COMP_BG
    popup_window_comp_bg_update(thiz);
#endif
    popup_info->runtime.force_update = false;
    return 0;
}
