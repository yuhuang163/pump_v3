/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define TRAVEL_LOCK_TITLE_COLOR                           (0xFFFFFF)
#define TRAVEL_LOCK_ARC_DONE_COLOR                        (0xFFFFFF)
/// 长按进入后圆弧从 0 起、3s 跑满；数字仍按 5s 倒计时
#define TRAVEL_LOCK_COUNTDOWN_TOTAL_SEC                   (5)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static popup_window_text_info_t travel_lock_infos[POPUP_WINDOW_TEXT_COUNT] = {
    {TRAVEL_LOCK_TITLE_TEXT, 0, TRAVEL_LOCK_TITLE_Y,
        UI_FONT_TITLE, LV_OPA_100, TRAVEL_LOCK_TITLE_COLOR,
        TRAVEL_LOCK_TITLE_LINE_SPACE},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_travel_lock_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type, uint32_t target_time)
{
    popup_window_disp_info_t *info = popup_window_info_get();
    ui_progress_arc_style_t style;

    EXCEPTION_CHECK_VOID(info);
    switch (popup_type) {
        case ID_POPUP_UI_TRAVEL_LOCK_COUNTDOWN:
            popup_window_mask_layer_opaque_update(thiz);
            popup_window_text_update(thiz, travel_lock_infos);
            ui_progress_arc_style_init(&style, TRAVEL_LOCK_ARC_DONE_COLOR);
            style.show_countdown = true;
            ui_progress_arc_time_update(&info->progress_arc, info->root.obj, &style,
                info->runtime.start_time, target_time,
                TRAVEL_LOCK_COUNTDOWN_TOTAL_SEC, info->runtime.force_update);
            popup_window_display_complete(thiz, popup_type);
            break;
        default:
            break;
    }
}
