/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define THEME_RESELECT_TEXT_COLOR                              (0xFFFFFF)
#define THEME_RESELECT_TEXT_Y                                  (186)

#define THEME_RESELECT_SPIN_ARC_SIZE                           (48)
#define THEME_RESELECT_SPIN_ARC_WIDTH                          (4)
#define THEME_RESELECT_SPIN_ARC_BG_COLOR                       (0xFFFFFF)
#define THEME_RESELECT_SPIN_ARC_BG_OPA                         (38)
#define THEME_RESELECT_SPIN_ARC_COLOR                          (0xBD4060)
#define THEME_RESELECT_SPIN_ARC_LOOP_MS_PER_DEG                (3)
#define THEME_RESELECT_SPIN_ARC_ANGLE_SPAN                     (90)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static ui_popup_text_info_t theme_reselect_infos[UI_POPUP_TEXT_COUNT] = {
    { "Restore factory settings\nto reselect", 0, THEME_RESELECT_TEXT_Y,
        &lv_font_aeonik_soft_26_medium, LV_OPA_100, THEME_RESELECT_TEXT_COLOR, 4},
};

const static ui_popup_spin_arc_info_t theme_reselect_arc_cfg = {
    .size = THEME_RESELECT_SPIN_ARC_SIZE,
    .width = THEME_RESELECT_SPIN_ARC_WIDTH,
    .bg_color = THEME_RESELECT_SPIN_ARC_BG_COLOR,
    .bg_opa = THEME_RESELECT_SPIN_ARC_BG_OPA,
    .color = THEME_RESELECT_SPIN_ARC_COLOR,
    .loop_ms_per_deg = THEME_RESELECT_SPIN_ARC_LOOP_MS_PER_DEG,
    .angle_span = THEME_RESELECT_SPIN_ARC_ANGLE_SPAN,
    .align = LV_ALIGN_TOP_MID,
    .x_offset = 0,
    .y_offset = 120,
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_popup_theme_reselect_update(ui_manager_context_t *thiz)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_spin_arc_update(thiz, &theme_reselect_arc_cfg);
    ui_popup_text_update(thiz, theme_reselect_infos);
    ui_popup_display_complete(thiz, ID_POPUP_UI_THEME_RESELECT);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_popup_theme_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    if (popup_type != ID_POPUP_UI_THEME_RESELECT) {
        return;
    }
    ui_popup_theme_reselect_update(thiz);
}
