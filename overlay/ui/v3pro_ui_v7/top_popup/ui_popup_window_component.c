/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POPUP_WINDOW_STATUS_ARC_SIZE                               (360)
#define POPUP_WINDOW_STATUS_ARC_WIDTH                              (12)
#define POPUP_WINDOW_STATUS_ARC_COLOR                              (0xBD4060)
#define POPUP_WINDOW_STATUS_ARC_START_ANGLE                        (270)
#define POPUP_WINDOW_STATUS_ARC_FULL_SWEEP                         (360)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void popup_window_text_hide(uint8_t index)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL || index >= POPUP_WINDOW_TEXT_COUNT) {
        return;
    }
    if (info->label.label[index] != NULL && lv_obj_is_valid(info->label.label[index])) {
        lv_obj_add_flag(info->label.label[index], LV_OBJ_FLAG_HIDDEN);
    }
}

static void popup_window_icon_hide(void)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL) {
        return;
    }
    if (info->icon.obj != NULL && lv_obj_is_valid(info->icon.obj)) {
        lv_obj_add_flag(info->icon.obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void popup_window_combo_key_icon_hide(lv_obj_t *icon)
{
    if (icon != NULL && lv_obj_is_valid(icon)) {
        lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
    }
}

static void popup_window_combo_key_icons_hide(void)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL) {
        return;
    }
    popup_window_combo_key_icon_hide(info->combo_key.left);
    popup_window_combo_key_icon_hide(info->combo_key.add);
    popup_window_combo_key_icon_hide(info->combo_key.right);
}

static void popup_window_arc_hide(void)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL) {
        return;
    }
    if (info->arc.obj != NULL && lv_obj_is_valid(info->arc.obj)) {
        lv_obj_add_flag(info->arc.obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void popup_window_number_hide(void)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL) {
        return;
    }
    if (info->countdown.label != NULL && lv_obj_is_valid(info->countdown.label)) {
        lv_obj_add_flag(info->countdown.label, LV_OBJ_FLAG_HIDDEN);
    }
}

static void popup_window_mask_hide(void)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL) {
        return;
    }
    if (info->mask.layer != NULL && lv_obj_is_valid(info->mask.layer)) {
        lv_obj_add_flag(info->mask.layer, LV_OBJ_FLAG_HIDDEN);
    }
}

static void popup_window_progress_arc_hide(void)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    if (info == NULL) {
        return;
    }
    ui_progress_arc_hide(&info->progress_arc);
    ui_progress_arc_countdown_hide(&info->progress_arc);
}

void popup_window_root_init(ui_manager_context_t *thiz)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    (void)thiz;
    EXCEPTION_CHECK_VOID(info);
    if (info->root.obj == NULL || !lv_obj_is_valid(info->root.obj)) {
        info->root.obj = lv_obj_create(lv_layer_top());
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

void popup_window_components_hide_all(void)
{
    popup_window_mask_hide();
    popup_window_arc_hide();
    popup_window_number_hide();
    popup_window_progress_arc_hide();
    popup_window_icon_hide();
    popup_window_combo_key_icons_hide();
    for (uint8_t i = 0; i < POPUP_WINDOW_TEXT_COUNT; i++) {
        popup_window_text_hide(i);
    }
}

void popup_window_text_update(ui_manager_context_t *thiz, const popup_window_text_info_t *text_info)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    EXCEPTION_CHECK_VOID(text_info);
    EXCEPTION_CHECK_VOID(info);
    (void)thiz;
    if (info->runtime.force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < POPUP_WINDOW_TEXT_COUNT; i++) {
        if (text_info[i].name == NULL || text_info[i].name[0] == '\0') {
            popup_window_text_hide(i);
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
        lv_obj_align(info->label.label[i], LV_ALIGN_TOP_MID, text_info[i].x_offset, text_info[i].y_offset);
        lv_obj_clear_flag(info->label.label[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void popup_window_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset,
    res_theme_type_e theme_type)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    EXCEPTION_CHECK_VOID(info);
    if (info->runtime.force_update == false) {
        return;
    }

    if (info->icon.obj == NULL || !lv_obj_is_valid(info->icon.obj)) {
        info->icon.obj = lv_image_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->icon.obj);
    }

    ui_img_baj_theme_set_src(thiz, info->icon.obj, img_id,
        theme_type, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(info->icon.obj, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_clear_flag(info->icon.obj, LV_OBJ_FLAG_HIDDEN);
}

static void popup_window_combo_key_icon_update(ui_manager_context_t *thiz,
    lv_obj_t **icon, uint16_t img_id, int16_t x, int16_t y)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    EXCEPTION_CHECK_VOID(info && icon);
    if (*icon == NULL || !lv_obj_is_valid(*icon)) {
        *icon = lv_image_create(info->root.obj);
        EXCEPTION_CHECK_VOID(*icon);
    }
    ui_img_baj_theme_set_src(thiz, *icon, img_id,
        RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(*icon, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_clear_flag(*icon, LV_OBJ_FLAG_HIDDEN);
}

void popup_window_combo_key_icons_update(ui_manager_context_t *thiz)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    EXCEPTION_CHECK_VOID(info);
    if (info->runtime.force_update == false) {
        return;
    }

    popup_window_combo_key_icon_update(thiz, &info->combo_key.left,
        BASE_RES_ID_STIMULATION_KEY_ICON,
        TRAVEL_LOCK_STIM_KEY_ICON_X, TRAVEL_LOCK_STIM_KEY_ICON_Y);
    popup_window_combo_key_icon_update(thiz, &info->combo_key.add,
        BASE_RES_ID_ADD_ICON,
        TRAVEL_LOCK_ADD_ICON_X, TRAVEL_LOCK_ADD_ICON_Y);
    popup_window_combo_key_icon_update(thiz, &info->combo_key.right,
        BASE_RES_ID_GUIDE_RIGHT_KEY_ICON,
        TRAVEL_LOCK_EXPR_KEY_ICON_X, TRAVEL_LOCK_EXPR_KEY_ICON_Y);
}

static void popup_window_mask_layer_style_update(ui_manager_context_t *thiz, lv_opa_t opa)
{
    popup_window_disp_info_t *info = popup_window_info_get();

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
    lv_obj_set_style_bg_opa(info->mask.layer, opa, 0);
    lv_obj_set_style_border_width(info->mask.layer, 0, 0);
    lv_obj_set_style_pad_all(info->mask.layer, 0, 0);
    lv_obj_clear_flag(info->mask.layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(info->mask.layer, LV_OBJ_FLAG_HIDDEN);
}

void popup_window_mask_layer_update(ui_manager_context_t *thiz)
{
    popup_window_mask_layer_style_update(thiz, 190);
}

void popup_window_mask_layer_opaque_update(ui_manager_context_t *thiz)
{
    popup_window_mask_layer_style_update(thiz, LV_OPA_COVER);
}

void popup_window_arc_update(ui_manager_context_t *thiz)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    EXCEPTION_CHECK_VOID(info);
    (void)thiz;
    uint16_t ring_progress = get_timestamp_ms(info->runtime.start_time) / POPUP_WINDOW_STATUS_DISPLAY_STEP_TIME;
    if (info->runtime.force_update == false && info->arc.ring_progress == ring_progress) {
        return;
    }

    info->arc.ring_progress = ring_progress;
    if (info->arc.obj == NULL || !lv_obj_is_valid(info->arc.obj)) {
        info->arc.obj = lv_arc_create(info->root.obj);
        EXCEPTION_CHECK_VOID(info->arc.obj);
        lv_obj_set_size(info->arc.obj, POPUP_WINDOW_STATUS_ARC_SIZE, POPUP_WINDOW_STATUS_ARC_SIZE);
        lv_obj_center(info->arc.obj);
        lv_arc_set_bg_angles(info->arc.obj, 0, POPUP_WINDOW_STATUS_ARC_SIZE);
        lv_arc_set_angles(info->arc.obj, 0, 0);
        lv_arc_set_rotation(info->arc.obj, POPUP_WINDOW_STATUS_ARC_START_ANGLE);
        lv_obj_set_style_arc_color(info->arc.obj, lv_color_hex(POPUP_WINDOW_STATUS_ARC_COLOR), LV_PART_MAIN);
        lv_obj_set_style_arc_width(info->arc.obj, POPUP_WINDOW_STATUS_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(info->arc.obj, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_color(info->arc.obj, lv_color_hex(POPUP_WINDOW_STATUS_ARC_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(info->arc.obj, POPUP_WINDOW_STATUS_ARC_WIDTH, LV_PART_INDICATOR);
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
        end_angle = (POPUP_WINDOW_STATUS_ARC_FULL_SWEEP * ring_progress) / info->arc.ring_progress_max;
    }
    lv_arc_set_angles(info->arc.obj, 0, end_angle);
}

void popup_window_num_update(ui_manager_context_t *thiz, int16_t x_offset, int16_t y_offset)
{
    popup_window_disp_info_t *info = popup_window_info_get();

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
    // show_num = 2; // for test
    snprintf(num_buf, sizeof(num_buf), "%d", show_num == 0 ? 1 : show_num);
    lv_label_set_text(info->countdown.label, num_buf);
    lv_obj_set_style_text_color(info->countdown.label, lv_color_white(), 0);
    lv_obj_set_style_text_font(info->countdown.label, UI_FONT_HINT, 0);
    lv_obj_set_style_text_align(info->countdown.label, LV_TEXT_ALIGN_CENTER, 0);
    if (x_offset == 0) {
        lv_obj_align(info->countdown.label, LV_ALIGN_TOP_MID, x_offset, y_offset);
    } else {
        lv_obj_align(info->countdown.label, LV_ALIGN_TOP_LEFT, x_offset, y_offset);
    }
    info->countdown.shown_value = show_num;
    lv_obj_clear_flag(info->countdown.label, LV_OBJ_FLAG_HIDDEN);
}

void popup_window_tip_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const popup_window_text_info_t *text_info, uint16_t img_id, uint16_t icon_y,
    res_theme_type_e theme_type)
{
    popup_window_mask_layer_update(thiz);
    popup_window_text_update(thiz, text_info);
    popup_window_icon_update(thiz, img_id, icon_y, theme_type);
    popup_window_display_complete(thiz, popup_type);
}

void popup_window_static_tip_ui_update(ui_manager_context_t *thiz,
    const popup_window_text_info_t *text_info, uint16_t img_id, uint16_t icon_y,
    res_theme_type_e theme_type)
{
    popup_window_mask_layer_update(thiz);
    popup_window_text_update(thiz, text_info);
    popup_window_icon_update(thiz, img_id, icon_y, theme_type);
}

void popup_window_arc_countdown_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type,
    const popup_window_text_info_t *text_info, int16_t x_offset, int16_t y_offset)
{
    popup_window_mask_layer_update(thiz);
    if (text_info != NULL) {
        popup_window_text_update(thiz, text_info);
    }
    popup_window_arc_update(thiz);
    popup_window_num_update(thiz, x_offset, y_offset);
    popup_window_display_complete(thiz, popup_type);
}

void popup_window_progress_arc_countdown_ui_update(ui_manager_context_t *thiz,
    popup_ui_id_e popup_type, const popup_window_text_info_t *text_info, uint32_t done_color)
{
    popup_window_disp_info_t *info = popup_window_info_get();
    ui_progress_arc_style_t style;
    uint8_t countdown_max;

    EXCEPTION_CHECK_VOID(info);
    popup_window_mask_layer_opaque_update(thiz);
    if (text_info != NULL) {
        popup_window_text_update(thiz, text_info);
    }

    ui_progress_arc_style_init(&style, done_color);
    style.show_countdown = true;
    countdown_max = (info->runtime.target_time > 0) ?
        (uint8_t)(info->runtime.target_time / 1000) : 1;
    if (countdown_max < 1) {
        countdown_max = 1;
    }
    ui_progress_arc_time_update(&info->progress_arc, info->root.obj, &style,
        info->runtime.start_time, info->runtime.target_time,
        countdown_max, info->runtime.force_update);
    popup_window_display_complete(thiz, popup_type);
}

void popup_window_display_complete(ui_manager_context_t *thiz, uint16_t popup_type)
{
    popup_window_disp_info_t *info = popup_window_info_get();

    EXCEPTION_CHECK_VOID(info);
    uint32_t time_diff = get_timestamp_ms(info->runtime.start_time) + POPUP_WINDOW_STATUS_DISPLAY_STEP_TIME;
    if (time_diff >= info->runtime.target_time) {
        ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
        EXCEPTION_CHECK_VOID(ani);
        if (ani->progress_cb == NULL) {
            return;
        }
        LOG_UI("%s display complete", popup_window_name_get(thiz, popup_type));
        ani->progress_cb(thiz, popup_type, ANI_POPUP_DISPLAY_FINISH);
    }
}

typedef struct {
    uint16_t popup_type;
    const char *name;
} popup_window_name_info_t;

const static popup_window_name_info_t popup_window_name_infos[] = {
    { ID_POPUP_UI_BT_CONNECT, "ble connect" },
    { ID_POPUP_UI_BT_DISCONNECT, "ble disconnect" },
    { ID_POPUP_UI_BT_CONNECTING, "ble connecting" },
    { ID_POPUP_UI_DEVICE_BIND, "device bind" },
    { ID_POPUP_UI_DEVICE_UNBIND, "device unbind" },
    { ID_POPUP_UI_TRAVEL_LOCK_COUNTDOWN, "travel lock countdown" },
    { ID_POPUP_UI_LOW_POWER, "low power" },
};

const char *popup_window_name_get(ui_manager_context_t *thiz, uint16_t popup_type)
{
    (void)thiz;

    for (uint8_t i = 0; i < sizeof(popup_window_name_infos) / sizeof(popup_window_name_infos[0]); i++) {
        if (popup_window_name_infos[i].popup_type == popup_type) {
            return popup_window_name_infos[i].name;
        }
    }
    return "Unknown";
}
