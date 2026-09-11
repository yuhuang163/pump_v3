/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_REST_MODE_ADJUST_TIP_TITLE_COLOR                  (0xFFFFFF)
#define PUMP_REST_MODE_ADJUST_TIP_COUNTDOWN_Y                  (324)
#define PUMP_REST_MODE_ADJUST_TIP_COUNTDOWN_COLOR              (0x999999)


/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_popup_pump_rest_mode_adjust_tip_countdown_update(ui_manager_context_t *thiz)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();
    char num_buf[8] = {0};
    uint8_t num;
    uint8_t show_num;

    EXCEPTION_CHECK_VOID(info);
    (void)thiz;
    if (info->countdown.label == NULL || !lv_obj_is_valid(info->countdown.label)) {
        info->countdown.label = lv_label_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->countdown.label);
    }

    num = get_timestamp_ms(info->runtime.start_time) / 1000;
    show_num = (num > info->countdown.max_value) ? 0 : (info->countdown.max_value - num);
    if (info->runtime.force_update == false &&
        info->countdown.shown_value == show_num) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), UI_STR_COUNTDOWN_SEC_FMT,
        (unsigned)(show_num == 0 ? 1 : show_num));
    lv_label_set_text(info->countdown.label, num_buf);
    lv_obj_set_style_text_color(info->countdown.label,
        lv_color_hex(PUMP_REST_MODE_ADJUST_TIP_COUNTDOWN_COLOR), 0);
    lv_obj_set_style_text_font(info->countdown.label, UI_FONT_HINT, 0);
    lv_obj_set_style_text_align(info->countdown.label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(info->countdown.label, LV_ALIGN_TOP_MID, 0, PUMP_REST_MODE_ADJUST_TIP_COUNTDOWN_Y);
    info->countdown.shown_value = show_num;
    lv_obj_clear_flag(info->countdown.label, LV_OBJ_FLAG_HIDDEN);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_pump_rest_mode_adjust_tip_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    const ui_popup_text_info_t pump_rest_mode_adjust_tip_infos[UI_POPUP_TEXT_COUNT] = {
        { PUMP_REST_MODE_ADJUST_TIP_TEXT, 0, PUMP_REST_MODE_ADJUST_TIP_TITLE_Y,
            UI_FONT_TITLE, LV_OPA_100, PUMP_REST_MODE_ADJUST_TIP_TITLE_COLOR,
            PUMP_REST_MODE_ADJUST_TIP_TITLE_LINE_SPACE},
    };

    if (popup_type != ID_POPUP_UI_PUMP_REST_MODE_ADJUST_TIP) {
        return;
    }

    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, pump_rest_mode_adjust_tip_infos);
    ui_popup_single_icon_update(thiz, BASE_RES_ID_FORBIDDEN_TIP_ICON, PUMP_REST_MODE_ADJUST_TIP_ICON_Y);
    ui_popup_pump_rest_mode_adjust_tip_countdown_update(thiz);
    ui_popup_display_complete(thiz, ID_POPUP_UI_PUMP_REST_MODE_ADJUST_TIP);
}
