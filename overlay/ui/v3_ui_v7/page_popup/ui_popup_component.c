/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_internal.h" // IWYU pragma: keep
#include <stdint.h>

/*
* DEFINES
*****************************************************************************************
*/
#define UI_POPUP_STATUS_ARC_SIZE                                   (360)
#define UI_POPUP_STATUS_ARC_WIDTH                                  (12)
#define UI_POPUP_STATUS_ARC_COLOR                                  (0xBD4060)
#define UI_POPUP_STATUS_ARC_START_ANGLE                            (270)
#define UI_POPUP_STATUS_ARC_FULL_SWEEP                             (360)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_popup_text_hide(uint8_t index)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    if (info == NULL || index >= UI_POPUP_TEXT_COUNT) {
        return;
    }
    if (info->label.label[index] != NULL && lv_obj_is_valid(info->label.label[index])) {
        lv_obj_add_flag(info->label.label[index], LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_popup_icon_hide(uint8_t index)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    if (info == NULL || index >= UI_POPUP_ICON_COUNT) {
        return;
    }
    if (info->icon.obj[index] != NULL && lv_obj_is_valid(info->icon.obj[index])) {
        lv_obj_add_flag(info->icon.obj[index], LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_popup_arc_hide(void)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    if (info == NULL) {
        return;
    }
    if (info->arc.obj != NULL && lv_obj_is_valid(info->arc.obj)) {
        lv_obj_add_flag(info->arc.obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_popup_number_hide(void)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    if (info == NULL) {
        return;
    }
    if (info->countdown.label != NULL && lv_obj_is_valid(info->countdown.label)) {
        lv_obj_add_flag(info->countdown.label, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_popup_mask_hide(void)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    if (info == NULL) {
        return;
    }
    if (info->mask.layer != NULL && lv_obj_is_valid(info->mask.layer)) {
        lv_obj_add_flag(info->mask.layer, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_popup_root_init(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    (void)thiz;
    EXCEPTION_CHECK_VOID(parent);
    EXCEPTION_CHECK_VOID(info);
    if (info->root.obj == NULL || !lv_obj_is_valid(info->root.obj)) {
        info->root.obj = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(info->root.obj);
        lv_obj_set_size(info->root.obj, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(info->root.obj, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(info->root.obj, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(info->root.obj, 0, 0);
        lv_obj_set_style_pad_all(info->root.obj, 0, 0);
        lv_obj_align(info->root.obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(info->root.obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(info->root.obj, LV_OBJ_FLAG_SCROLLABLE);
    }
}

void ui_popup_components_hide_all(void)
{
    ui_popup_mask_hide();
    ui_popup_arc_hide();
    ui_popup_number_hide();
    for (uint8_t i = 0; i < UI_POPUP_ICON_COUNT; i++) {
        ui_popup_icon_hide(i);
    }
    for (uint8_t i = 0; i < UI_POPUP_TEXT_COUNT; i++) {
        ui_popup_text_hide(i);
    }
}

void ui_popup_text_update(ui_manager_context_t *thiz, const ui_popup_text_info_t *text_info)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    EXCEPTION_CHECK_VOID(text_info);
    EXCEPTION_CHECK_VOID(info);
    (void)thiz;
    if (info->runtime.force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < UI_POPUP_TEXT_COUNT; i++) {
        if (text_info[i].name == NULL || text_info[i].name[0] == '\0') {
            ui_popup_text_hide(i);
            continue;
        }
        if (info->label.label[i] == NULL || !lv_obj_is_valid(info->label.label[i])) {
            info->label.label[i] = lv_label_create(info->root.obj);
            EXCEPTION_CHECK_VOID(info->label.label[i]);
        }
        lv_label_set_text(info->label.label[i], text_info[i].name);
        lv_obj_set_style_text_color(info->label.label[i], lv_color_hex(text_info[i].color), 0);
        lv_obj_set_style_text_font(info->label.label[i], text_info[i].font, 0);
        lv_obj_set_style_text_opa(info->label.label[i], text_info[i].text_opa, 0);
        lv_obj_set_style_text_line_space(info->label.label[i], text_info[i].line_space, 0);
        lv_obj_set_style_text_align(info->label.label[i], LV_TEXT_ALIGN_CENTER, 0);
        if (text_info[i].x_offset == 0) {
            lv_obj_align(info->label.label[i], LV_ALIGN_TOP_MID, text_info[i].x_offset, text_info[i].y_offset);
        } else {
            lv_obj_align(info->label.label[i], LV_ALIGN_TOP_LEFT, text_info[i].x_offset, text_info[i].y_offset);
        }
        lv_obj_clear_flag(info->label.label[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_popup_icon_update(ui_manager_context_t *thiz, uint8_t index, uint16_t img_id,
    int16_t x_offset, uint16_t y_offset)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    if (info == NULL || index >= UI_POPUP_ICON_COUNT) {
        return;
    }
    if (info->runtime.force_update == false) {
        return;
    }

    if (info->icon.obj[index] == NULL || !lv_obj_is_valid(info->icon.obj[index])) {
        info->icon.obj[index] = lv_image_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->icon.obj[index]);
    }

    ui_img_baj_theme_set_src(thiz, info->icon.obj[index], img_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    if (x_offset == 0) {
        lv_obj_align(info->icon.obj[index], LV_ALIGN_TOP_MID, 0, y_offset);
    } else {
        lv_obj_align(info->icon.obj[index], LV_ALIGN_TOP_LEFT, x_offset, y_offset);
    }
    lv_obj_clear_flag(info->icon.obj[index], LV_OBJ_FLAG_HIDDEN);
}

void ui_popup_single_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset)
{
    ui_popup_icon_update(thiz, 0, img_id, 0, y_offset);
}

void ui_popup_mask_layer_update(ui_manager_context_t *thiz)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    EXCEPTION_CHECK_VOID(info);
    (void)thiz;
    if (info->runtime.force_update == false) {
        return;
    }
    if (info->mask.layer == NULL || !lv_obj_is_valid(info->mask.layer)) {
        info->mask.layer = lv_obj_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->mask.layer);
    }
    lv_obj_set_size(info->mask.layer, LV_PCT(100), LV_PCT(100));
    lv_obj_center(info->mask.layer);
    lv_obj_set_style_bg_color(info->mask.layer, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(info->mask.layer, 190, 0);
    lv_obj_set_style_border_width(info->mask.layer, 0, 0);
    lv_obj_set_style_pad_all(info->mask.layer, 0, 0);
    lv_obj_clear_flag(info->mask.layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(info->mask.layer, LV_OBJ_FLAG_HIDDEN);
}

void ui_popup_arc_update(ui_manager_context_t *thiz)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    EXCEPTION_CHECK_VOID(info);
    (void)thiz;
    uint16_t ring_progress = get_timestamp_ms(info->runtime.start_time) / UI_POPUP_STATUS_DISPLAY_STEP_TIME;
    if (info->runtime.force_update == false && info->arc.ring_progress == ring_progress) {
        return;
    }

    info->arc.ring_progress = ring_progress;
    if (info->arc.obj == NULL || !lv_obj_is_valid(info->arc.obj)) {
        info->arc.obj = lv_arc_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->arc.obj);
        lv_obj_set_size(info->arc.obj, UI_POPUP_STATUS_ARC_SIZE, UI_POPUP_STATUS_ARC_SIZE);
        lv_obj_center(info->arc.obj);
        lv_arc_set_bg_angles(info->arc.obj, 0, UI_POPUP_STATUS_ARC_SIZE);
        lv_arc_set_angles(info->arc.obj, 0, 0);
        lv_arc_set_rotation(info->arc.obj, UI_POPUP_STATUS_ARC_START_ANGLE);
        lv_obj_set_style_arc_color(info->arc.obj, lv_color_hex(UI_POPUP_STATUS_ARC_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_width(info->arc.obj, UI_POPUP_STATUS_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_color(info->arc.obj, lv_color_hex(UI_POPUP_STATUS_ARC_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(info->arc.obj, UI_POPUP_STATUS_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(info->arc.obj, NULL, LV_PART_KNOB);
    }
    lv_obj_clear_flag(info->arc.obj, LV_OBJ_FLAG_HIDDEN);

    if (ring_progress >= info->arc.ring_progress_max) {
        lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(info->arc.obj, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (UI_POPUP_STATUS_ARC_FULL_SWEEP * ring_progress) / info->arc.ring_progress_max;
    }
    lv_arc_set_angles(info->arc.obj, 0, end_angle);
}

void ui_popup_spin_arc_update(ui_manager_context_t *thiz, const ui_popup_spin_arc_info_t *cfg)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    EXCEPTION_CHECK_VOID(info);
    EXCEPTION_CHECK_VOID(cfg);
    (void)thiz;
    if (info->arc.obj == NULL || !lv_obj_is_valid(info->arc.obj)) {
        info->arc.obj = lv_arc_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->arc.obj);
        lv_obj_set_size(info->arc.obj, cfg->size, cfg->size);
        lv_arc_set_bg_angles(info->arc.obj, 0, 360);
        lv_obj_set_style_arc_color(info->arc.obj, lv_color_hex(cfg->bg_color), LV_PART_MAIN);
        lv_obj_set_style_arc_width(info->arc.obj, cfg->width, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(info->arc.obj, cfg->bg_opa, LV_PART_MAIN);
        lv_obj_set_style_arc_color(info->arc.obj, lv_color_hex(cfg->color), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(info->arc.obj, cfg->width, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_remove_style(info->arc.obj, NULL, LV_PART_KNOB);
    }
    lv_obj_align(info->arc.obj, cfg->align, cfg->x_offset, cfg->y_offset);
    lv_obj_clear_flag(info->arc.obj, LV_OBJ_FLAG_HIDDEN);

    uint32_t elapsed = get_timestamp_ms(info->runtime.start_time);
    uint16_t start_angle = (uint16_t)((elapsed / cfg->loop_ms_per_deg) % 360);
    uint16_t end_angle = start_angle + cfg->angle_span;
    lv_arc_set_angles(info->arc.obj, start_angle, end_angle);
    lv_obj_invalidate(info->arc.obj);
}

void ui_popup_num_update(ui_manager_context_t *thiz, int16_t y_offset)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    EXCEPTION_CHECK_VOID(info);
    (void)thiz;
    if (info->countdown.label == NULL || !lv_obj_is_valid(info->countdown.label)) {
        info->countdown.label = lv_label_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->countdown.label);
    }
    char num_buf[8] = {0};
    uint8_t num = get_timestamp_ms(info->runtime.start_time) / 1000;
    uint8_t show_num = (num > info->countdown.max_value ? 0 : info->countdown.max_value - num);
    if (info->runtime.force_update == false &&
        info->countdown.shown_value == show_num) {
        return;
    }
    snprintf(num_buf, sizeof(num_buf), "%d", show_num == 0 ? 1 : show_num);
    lv_label_set_text(info->countdown.label, num_buf);
    lv_obj_set_style_text_color(info->countdown.label, lv_color_white(), 0);
    lv_obj_set_style_text_font(info->countdown.label, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(info->countdown.label, LV_TEXT_ALIGN_CENTER, 0);
    if (y_offset == 0) {
        lv_obj_align(info->countdown.label, LV_ALIGN_CENTER, 0, 0);
    } else {
        lv_obj_align(info->countdown.label, LV_ALIGN_TOP_MID, 0, y_offset);
    }
    info->countdown.shown_value = show_num;
    lv_obj_clear_flag(info->countdown.label, LV_OBJ_FLAG_HIDDEN);
}

void ui_popup_tip_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const ui_popup_text_info_t *text_info, uint16_t img_id, uint16_t icon_y)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, text_info);
    ui_popup_single_icon_update(thiz, img_id, icon_y);
    ui_popup_display_complete(thiz, popup_type);
}

void ui_popup_arc_countdown_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const ui_popup_text_info_t *text_info)
{
    ui_popup_mask_layer_update(thiz);
    ui_popup_text_update(thiz, text_info);
    ui_popup_arc_update(thiz);
    ui_popup_display_complete(thiz, popup_type);
}

void ui_popup_display_complete(ui_manager_context_t *thiz, uint16_t popup_type)
{
    ui_popup_disp_info_t *info = ui_popup_info_get();

    EXCEPTION_CHECK_VOID(info);
    uint32_t time_diff = get_timestamp_ms(info->runtime.start_time) + UI_POPUP_STATUS_DISPLAY_STEP_TIME;
    if (time_diff >= info->runtime.target_time) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        LOG_UI("%s display complete", ui_popup_name_get(thiz, popup_type));
        ani->progress_cb(thiz, popup_type, ANI_POPUP_DISPLAY_FINISH);
    }
}

typedef struct {
    uint16_t popup_type;
    const char *name;
} ui_popup_name_info_t;

const static ui_popup_name_info_t ui_popup_name_infos[] = {
    { ID_POPUP_UI_PUMP_RUNING_30M_TIP, "pump Running 30 Minutes" },
};

const char *ui_popup_name_get(ui_manager_context_t *thiz, uint16_t popup_type)
{
    (void)thiz;

    for (uint8_t i = 0; i < sizeof(ui_popup_name_infos) / sizeof(ui_popup_name_infos[0]); i++) {
        if (ui_popup_name_infos[i].popup_type == popup_type) {
            return ui_popup_name_infos[i].name;
        }
    }
    return "Unknown";
}

uint16_t ui_popup_target_time_get(popup_ui_id_e type)
{
    switch (type) {
        case ID_POPUP_UI_PUMP_RUNING_30M_TIP:
            return 3000;
        default:
            return 3000;
    }
}
