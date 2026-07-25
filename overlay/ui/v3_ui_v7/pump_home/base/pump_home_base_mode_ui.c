/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "pump_home_base_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_HOME_MODE_TITLE_COLOR                 (0xBD4060)
#define PUMP_HOME_MODE_NAME_COLOR                  (0xD36682)
#define PUMP_HOME_MODE_TITLE_Y                     (70)
#define PUMP_HOME_MODE_NAME_X                      (0)
#define PUMP_HOME_MODE_NAME_Y                      (155)

#define PUMP_HOME_SELECT_DOT_MAX                   (3)
#define PUMP_HOME_CAPSULE_Y                        (64)
#define PUMP_HOME_CAPSULE_W                        (142)
#define PUMP_HOME_CAPSULE_H                        (50)
#define PUMP_HOME_COMMON_CAPSULE_TITLE_Y           (10)

#define PUMP_HOME_BF_CAPSULE_Y                     (64)
#define PUMP_HOME_BF_CAPSULE_H                     (50)
#define PUMP_HOME_BF_CAPSULE_W_PAD                 (32)
#define PUMP_HOME_BF_CAPSULE_TITLE_Y               (14)

#define PUMP_HOME_CAPSULE_DOT_AREA_H               (14)
#define PUMP_HOME_CAPSULE_DOT_BOTTOM_PAD           (8)

#define PUMP_HOME_CAPSULE_BG_COLOR                 (0xBD4060)
#define PUMP_HOME_CAPSULE_TITLE_COLOR              (0xFFFFFF)

#define PUMP_HOME_DOT_SIZE                         (6)
#define PUMP_HOME_DOT_GAP                          (8)
#define PUMP_HOME_DOT_ACTIVE_COLOR                 (0xFFFFFF)
#define PUMP_HOME_DOT_INACTIVE_COLOR               (0xD58599)

#define PUMP_HOME_SELECT_WAVE_Y_OFFSET             (22)
#define PUMP_HOME_SELECT_CLOCK_Y_OFFSET            (22)
#define PUMP_HOME_SELECT_CLOCK_2SEG_Y_OFFSET       (8)
#define PUMP_HOME_SELECT_MODE_Y_OFFSET             (22)
#define PUMP_HOME_SELECT_REMAIN_Y_OFFSET           (22)

#define PUMP_HOME_SELECT_CAROUSEL_WAVE_Y_OFFSET        (22)
#define PUMP_HOME_SELECT_CAROUSEL_CLOCK_Y_OFFSET       (8)
#define PUMP_HOME_SELECT_CAROUSEL_CLOCK_2SEG_Y_OFFSET  (8)
#define PUMP_HOME_SELECT_CAROUSEL_MODE_Y_OFFSET        (22)
#define PUMP_HOME_SELECT_CAROUSEL_REMAIN_Y_OFFSET      (22)

#define PUMP_HOME_SELECT_ANIM_TIME_MS              (300)
#define PUMP_HOME_SELECT_ENTER_Y_OFFSET            (8)
#define PUMP_HOME_WAVE_Y                           (60)

#define PUMP_HOME_SELECT_TITLE_ACTIVE_Y              (82)
#define PUMP_HOME_SELECT_TITLE_INACTIVE_Y            (84)
#define PUMP_HOME_SELECT_TITLE_GAP                   (24)
#define PUMP_HOME_SELECT_CAROUSEL_SIDE_COLOR         (0xE89AAB)
#define PUMP_HOME_SELECT_SLOT_PREV                   0
#define PUMP_HOME_SELECT_SLOT_CUR                    1
#define PUMP_HOME_SELECT_SLOT_NEXT                   2

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static const char *pump_home_mode_type_name_get(uint8_t mode_type)
{
    switch (mode_type) {
        default:
        case PUMP_MODE_STIMULATION:
            return "Stimulation";
        case PUMP_MODE_LACTATION:
            return "Expression";
        case PUMP_MODE_MIX:
            return "Mixed";
        case PUMP_MODE_IDLE:
            return "Relax";
    }
}

static const char *pump_home_mode_common_name_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            return "Stimulation";
        case BREAST_PUMP_LACTATION_MODE:
            return "Expression";
        case BREAST_PUMP_MIX_MODE:
            return "Mixed";
        default:
            return "";
    }
}

static const char *pump_home_mode_bf_name_get(pump_home_base_ui_info_t *info, uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_MILK_BOOST_MODE:
            return "Milk Boost";
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return "Power Pumping";
        case BREAST_PUMP_CUSTOM_MODE: {
            static char custom_title[24];
            snprintf(custom_title, sizeof(custom_title), "Customize %s", info->mode.custom_id_str);
            return custom_title;
        }
        default:
            return "";
    }
}

static void pump_home_mode_obj_visible_set(lv_obj_t *obj, bool visible)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    if (visible) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void pump_home_mode_title_update(pump_home_base_ui_info_t *info, const char *title)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        info->mode.title = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.title);
        lv_obj_set_style_text_color(info->mode.title, lv_color_hex(PUMP_HOME_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.title, &lv_font_aeonik_soft_24_bold, 0);
    }

    if (title == NULL) {
        lv_obj_add_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.title, title);
    lv_obj_align(info->mode.title, LV_ALIGN_TOP_MID, 0, PUMP_HOME_MODE_TITLE_Y);
}

static void pump_home_mode_name_update(pump_home_base_ui_info_t *info, const char *name,
    uint16_t x_offset, uint16_t y_offset)
{
    if (info->mode.mode == NULL || !lv_obj_is_valid(info->mode.mode)) {
        info->mode.mode = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.mode);
        lv_obj_set_style_text_color(info->mode.mode, lv_color_hex(PUMP_HOME_MODE_NAME_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.mode, &lv_font_aeonik_soft_18_medium, 0);
    }

    if (name == NULL) {
        lv_obj_add_flag(info->mode.mode, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    info->mode.mode_name_x = x_offset;
    info->mode.mode_name_y = y_offset;
    lv_obj_clear_flag(info->mode.mode, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.mode, name);
    if (x_offset == 0) {
        lv_obj_align(info->mode.mode, LV_ALIGN_TOP_MID, 0,
            y_offset + info->mode.select_mode_y_offset);
    } else {
        lv_obj_align(info->mode.mode, LV_ALIGN_TOP_LEFT, x_offset,
            y_offset + info->mode.select_mode_y_offset);
    }
}

#if PUMP_HOME_MODE_TITLE_BLINK_ENABLE
static void pump_home_mode_title_blink_apply(pump_home_base_ui_info_t *info, bool blink_visible)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        return;
    }
    const bool visible = (info->mode.mode_blink_state == 0) || blink_visible;
    pump_home_mode_obj_visible_set(info->mode.title, visible);
}
#endif

static bool pump_home_mode_is_rhythm_mode(uint8_t mode_index)
{
    return mode_index == BREAST_PUMP_MILK_BOOST_MODE ||
        mode_index == BREAST_PUMP_PP_MILK_TRACK_MODE ||
        mode_index == BREAST_PUMP_CUSTOM_MODE;
}

static void pump_home_mode_apply_index_change(pump_home_base_ui_info_t *info)
{
    char rhythm_name[24] = {0};

    switch (info->mode.new_index) {
        default:
        case BREAST_PUMP_STIMULATION_MODE:
            pump_home_mode_title_update(info, "Stimulation");
            pump_home_mode_name_update(info, NULL, 0, 0);
            break;
        case BREAST_PUMP_LACTATION_MODE:
            pump_home_mode_title_update(info, "Expression");
            pump_home_mode_name_update(info, NULL, 0, 0);
            break;
        case BREAST_PUMP_MIX_MODE:
            pump_home_mode_title_update(info, "Mixed");
            pump_home_mode_name_update(info, NULL, 0, 0);
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            pump_home_mode_title_update(info, "Milk Boost");
            pump_home_mode_name_update(info,
                pump_home_mode_type_name_get(info->mode.new_mode_type),
                PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            pump_home_mode_title_update(info, "Power Pumping");
            pump_home_mode_name_update(info,
                pump_home_mode_type_name_get(info->mode.new_mode_type),
                PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            snprintf(rhythm_name, sizeof(rhythm_name), "Customize %s", info->mode.custom_id_str);
            pump_home_mode_title_update(info, rhythm_name);
            pump_home_mode_name_update(info,
                pump_home_mode_type_name_get(info->mode.new_mode_type),
                PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
            break;
    }

    info->mode.index = info->mode.new_index;
    info->mode.mode_type = info->mode.new_mode_type;
}

static void pump_home_mode_apply_type_change(pump_home_base_ui_info_t *info)
{
    if (!pump_home_mode_is_rhythm_mode(info->mode.new_index)) {
        info->mode.mode_type = info->mode.new_mode_type;
        return;
    }

    const char *new_name = pump_home_mode_type_name_get(info->mode.new_mode_type);
    if (info->mode.mode != NULL && lv_obj_is_valid(info->mode.mode)) {
        const char *cur_name = lv_label_get_text(info->mode.mode);
        if (cur_name != NULL && strcmp(cur_name, new_name) == 0) {
            info->mode.mode_type = info->mode.new_mode_type;
            return;
        }
    }

    pump_home_mode_name_update(info, new_name, PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
    info->mode.mode_type = info->mode.new_mode_type;
}

#if PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE
static bool pump_home_mode_is_bf_select_mode(uint8_t mode)
{
    return mode == BREAST_PUMP_MILK_BOOST_MODE ||
        mode == BREAST_PUMP_PP_MILK_TRACK_MODE ||
        mode == BREAST_PUMP_CUSTOM_MODE;
}

static bool pump_home_mode_custom_active_get(pump_home_base_ui_info_t *info)
{
    int activate_status = 0;

    if (info->ui == NULL) {
        return false;
    }
    ui_proc_pump_t *proc = (ui_proc_pump_t *)info->ui->priv.play_info.user_data;
    if (proc == NULL || proc->pump == NULL) {
        return false;
    }
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    return activate_status > 0;
}

static uint8_t pump_home_mode_common_dot_index_get(uint8_t mode)
{
    if (!PUMP_IS_COMMON_MODE(mode)) {
        return 0;
    }
    return (uint8_t)(mode - BREAST_PUMP_NORMAL_MODE_LOWER);
}

static uint8_t pump_home_mode_bf_dot_index_get(uint8_t mode)
{
    if (mode == BREAST_PUMP_MILK_BOOST_MODE) {
        return 0;
    }
    if (mode == BREAST_PUMP_PP_MILK_TRACK_MODE) {
        return 1;
    }
    if (mode == BREAST_PUMP_CUSTOM_MODE) {
        return 2;
    }
    return 0;
}

static uint8_t pump_home_mode_bf_dot_count_get(bool custom_active)
{
    return custom_active ? PUMP_HOME_SELECT_DOT_MAX : 2;
}

static uint8_t pump_home_mode_select_dot_index_get(pump_home_base_ui_info_t *info, uint8_t mode)
{
    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        return pump_home_mode_bf_dot_index_get(mode);
    }
    return pump_home_mode_common_dot_index_get(mode);
}

static lv_coord_t pump_home_mode_select_capsule_y_get(const pump_home_base_ui_info_t *info)
{
    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        return PUMP_HOME_BF_CAPSULE_Y;
    }
    return PUMP_HOME_CAPSULE_Y;
}

static bool pump_home_mode_select_ui_active(pump_mode_display_type_e display_type, uint8_t mode)
{
    if (display_type == PUMP_MODE_DISPLAY_NORMAL_MODE && PUMP_IS_COMMON_MODE(mode)) {
        return true;
    }
    if (display_type == PUMP_MODE_DISPLAY_BF_MODE && pump_home_mode_is_bf_select_mode(mode)) {
        return true;
    }
    return false;
}

static uint8_t pump_home_mode_select_capsule_style_get(pump_mode_display_type_e display_type,
    uint8_t mode)
{
    if (display_type == PUMP_MODE_DISPLAY_BF_MODE && pump_home_mode_is_bf_select_mode(mode)) {
        return PUMP_HOME_CAPSULE_STYLE_BF;
    }
    if (display_type == PUMP_MODE_DISPLAY_NORMAL_MODE && PUMP_IS_COMMON_MODE(mode)) {
        return PUMP_HOME_CAPSULE_STYLE_COMMON;
    }
    return PUMP_HOME_CAPSULE_STYLE_NONE;
}

static void pump_home_mode_select_layout_target_get(const pump_home_base_ui_info_t *info,
    bool shift_down, int16_t *wave_offset, int16_t *clock_offset, int16_t *clock_2seg_offset,
    int16_t *mode_offset, int16_t *remain_offset)
{
    uint8_t mode = info->mode.new_index;

    if (!shift_down) {
        *wave_offset = 0;
        *clock_offset = 0;
        *clock_2seg_offset = 0;
        *mode_offset = 0;
        *remain_offset = 0;
        return;
    }

#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
    *wave_offset = PUMP_HOME_SELECT_WAVE_Y_OFFSET;
    *clock_offset = PUMP_HOME_SELECT_CLOCK_Y_OFFSET;
    *clock_2seg_offset = PUMP_HOME_SELECT_CLOCK_2SEG_Y_OFFSET;
    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        *mode_offset = PUMP_HOME_SELECT_MODE_Y_OFFSET;
        *remain_offset = PUMP_HOME_SELECT_REMAIN_Y_OFFSET;
        return;
    }
    *mode_offset = 0;
    *remain_offset = 0;
#elif PUMP_HOME_MODE_SELECT_CAROUSEL_ENABLE
    *wave_offset = PUMP_HOME_SELECT_CAROUSEL_WAVE_Y_OFFSET;
    *clock_offset = PUMP_HOME_SELECT_CAROUSEL_CLOCK_Y_OFFSET;
    *clock_2seg_offset = PUMP_HOME_SELECT_CAROUSEL_CLOCK_2SEG_Y_OFFSET;
    switch (mode) {
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
            *mode_offset = PUMP_HOME_SELECT_CAROUSEL_MODE_Y_OFFSET;
            *remain_offset = PUMP_HOME_SELECT_CAROUSEL_REMAIN_Y_OFFSET;
            break;
        default:
            *mode_offset = 0;
            *remain_offset = 0;
            break;
    }
#endif
}

#if !PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
static uint8_t pump_home_mode_neighbor_next_get(const pump_home_base_ui_info_t *info, uint8_t mode)
{
    const bool custom_active = info->mode.custom_mode_active != 0;

    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        if (!PUMP_IS_BREASTFEEDING_MODE(mode)) {
            mode = BREAST_PUMP_EXPERT_MODE_DEFAULT;
        }
        if (mode == BREAST_PUMP_CUSTOM_MODE && !custom_active) {
            return BREAST_PUMP_EXPERT_MODE_LOWER;
        }
        const uint8_t upper = custom_active ?
            BREAST_PUMP_CUSTOM_MODE : BREAST_PUMP_PP_MILK_TRACK_MODE;
        return (uint8_t)mode < upper ? (uint8_t)(mode + 1) : BREAST_PUMP_EXPERT_MODE_LOWER;
    }

    if (!PUMP_IS_COMMON_MODE(mode)) {
        mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
    }
    return (uint8_t)mode < BREAST_PUMP_NORMAL_MODE_UPPER ?
        (uint8_t)(mode + 1) : BREAST_PUMP_NORMAL_MODE_LOWER;
}

static uint8_t pump_home_mode_neighbor_prev_get(const pump_home_base_ui_info_t *info, uint8_t mode)
{
    const bool custom_active = info->mode.custom_mode_active != 0;

    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        if (!PUMP_IS_BREASTFEEDING_MODE(mode)) {
            mode = BREAST_PUMP_EXPERT_MODE_DEFAULT;
        }
        if (mode <= BREAST_PUMP_EXPERT_MODE_LOWER) {
            return custom_active ?
                BREAST_PUMP_CUSTOM_MODE : BREAST_PUMP_PP_MILK_TRACK_MODE;
        }
        return (uint8_t)(mode - 1);
    }

    if (!PUMP_IS_COMMON_MODE(mode)) {
        mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
    }
    return (uint8_t)mode > BREAST_PUMP_NORMAL_MODE_LOWER ?
        (uint8_t)(mode - 1) : BREAST_PUMP_NORMAL_MODE_UPPER;
}

static int8_t pump_home_mode_select_slide_dir_get(const pump_home_base_ui_info_t *info)
{
    const uint8_t cur = info->mode.index;
    const uint8_t next = info->mode.new_index;

    if (next == pump_home_mode_neighbor_next_get(info, cur)) {
        return 1;
    }
    if (next == pump_home_mode_neighbor_prev_get(info, cur)) {
        return -1;
    }
    return 0;
}

static const char *pump_home_mode_select_name_get(pump_home_base_ui_info_t *info, uint8_t mode)
{
    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        return pump_home_mode_bf_name_get(info, mode);
    }
    return pump_home_mode_common_name_get(mode);
}

static void pump_home_mode_select_title_style_apply(lv_obj_t *label, bool active)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return;
    }

    lv_obj_set_style_text_color(label, lv_color_hex(active ?
        PUMP_HOME_MODE_TITLE_COLOR : PUMP_HOME_SELECT_CAROUSEL_SIDE_COLOR), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    if (active) {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_26_bold, 0);
    } else {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_22_semibold, 0);
    }
}

static lv_coord_t pump_home_mode_select_title_half_width_get(lv_obj_t *label)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return 0;
    }

    lv_obj_update_layout(label);
    const lv_coord_t width = lv_obj_get_width(label);
    return width > 0 ? (lv_coord_t)(width / 2) : 0;
}

static lv_coord_t pump_home_mode_select_carousel_step_get(pump_home_base_ui_info_t *info, int8_t dir)
{
    lv_obj_t *prev = info->mode.select_titles[PUMP_HOME_SELECT_SLOT_PREV];
    lv_obj_t *cur = info->mode.select_titles[PUMP_HOME_SELECT_SLOT_CUR];
    lv_obj_t *next = info->mode.select_titles[PUMP_HOME_SELECT_SLOT_NEXT];

    if (dir == 1) {
        return pump_home_mode_select_title_half_width_get(prev) +
            PUMP_HOME_SELECT_TITLE_GAP + pump_home_mode_select_title_half_width_get(cur);
    }
    if (dir == -1) {
        return pump_home_mode_select_title_half_width_get(cur) +
            PUMP_HOME_SELECT_TITLE_GAP + pump_home_mode_select_title_half_width_get(next);
    }
    return 0;
}

static void pump_home_mode_select_carousel_realign(pump_home_base_ui_info_t *info)
{
    const lv_coord_t slide = info->mode.select_title_slide_offset;
    lv_obj_t *labels[3] = {
        info->mode.select_titles[PUMP_HOME_SELECT_SLOT_PREV],
        info->mode.select_titles[PUMP_HOME_SELECT_SLOT_CUR],
        info->mode.select_titles[PUMP_HOME_SELECT_SLOT_NEXT],
    };
    lv_coord_t half_w[3];

    for (uint8_t i = 0; i < 3; i++) {
        half_w[i] = pump_home_mode_select_title_half_width_get(labels[i]);
    }

    const lv_coord_t center_x = lv_obj_get_width(info->root) / 2;
    const lv_coord_t x_mid = center_x - slide;
    lv_coord_t x_pos[3];

    x_pos[PUMP_HOME_SELECT_SLOT_CUR] = x_mid;
    x_pos[PUMP_HOME_SELECT_SLOT_PREV] = x_mid - half_w[PUMP_HOME_SELECT_SLOT_CUR] -
        PUMP_HOME_SELECT_TITLE_GAP - half_w[PUMP_HOME_SELECT_SLOT_PREV];
    x_pos[PUMP_HOME_SELECT_SLOT_NEXT] = x_mid + half_w[PUMP_HOME_SELECT_SLOT_CUR] +
        PUMP_HOME_SELECT_TITLE_GAP + half_w[PUMP_HOME_SELECT_SLOT_NEXT];

    for (uint8_t i = 0; i < 3; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            continue;
        }
        const lv_coord_t title_y = (i == PUMP_HOME_SELECT_SLOT_CUR) ?
            PUMP_HOME_SELECT_TITLE_ACTIVE_Y : PUMP_HOME_SELECT_TITLE_INACTIVE_Y;
        lv_obj_align(labels[i], LV_ALIGN_TOP_MID, x_pos[i] - center_x, title_y);
    }
}

static void pump_home_mode_select_carousel_create(pump_home_base_ui_info_t *info)
{
    for (uint8_t i = 0; i < 3; i++) {
        if (info->mode.select_titles[i] != NULL && lv_obj_is_valid(info->mode.select_titles[i])) {
            continue;
        }
        info->mode.select_titles[i] = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.select_titles[i]);
        pump_home_mode_select_title_style_apply(info->mode.select_titles[i],
            i == PUMP_HOME_SELECT_SLOT_CUR);
        lv_obj_add_flag(info->mode.select_titles[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void pump_home_mode_select_carousel_content_update_for_center(pump_home_base_ui_info_t *info,
    uint8_t center_mode)
{
    const uint8_t prev = pump_home_mode_neighbor_prev_get(info, center_mode);
    const uint8_t next = pump_home_mode_neighbor_next_get(info, center_mode);
    const uint8_t modes[3] = { prev, center_mode, next };

    pump_home_mode_select_carousel_create(info);
    for (uint8_t i = 0; i < 3; i++) {
        lv_obj_t *label = info->mode.select_titles[i];
        if (label == NULL) {
            continue;
        }
        lv_label_set_text(label, pump_home_mode_select_name_get(info, modes[i]));
        pump_home_mode_select_title_style_apply(label, i == PUMP_HOME_SELECT_SLOT_CUR);
    }
    pump_home_mode_select_carousel_realign(info);
}

static void pump_home_mode_select_carousel_content_update(pump_home_base_ui_info_t *info)
{
    pump_home_mode_select_carousel_content_update_for_center(info, info->mode.new_index);
}

static void pump_home_mode_select_carousel_slide_ready_cb(lv_anim_t *anim)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)lv_anim_get_user_data(anim);
    if (info == NULL) {
        return;
    }

    info->mode.select_title_slide_offset = 0;
    info->mode.select_slide_dir = 0;
    pump_home_mode_select_carousel_content_update(info);
}

static void pump_home_mode_select_carousel_slide_exec(void *var, int32_t v);

static void pump_home_mode_select_carousel_slide_stop(pump_home_base_ui_info_t *info)
{
    lv_anim_del(info, pump_home_mode_select_carousel_slide_exec);
    info->mode.select_title_slide_offset = 0;
    info->mode.select_slide_dir = 0;
}

static void pump_home_mode_select_carousel_visible_set(pump_home_base_ui_info_t *info, bool visible)
{
    for (uint8_t i = 0; i < 3; i++) {
        pump_home_mode_obj_visible_set(info->mode.select_titles[i], visible);
    }
}

static void pump_home_mode_select_carousel_slide_exec(void *var, int32_t v)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)var;
    info->mode.select_title_slide_offset = (int16_t)v;
    pump_home_mode_select_carousel_realign(info);
}

static void pump_home_mode_select_carousel_slide(pump_home_base_ui_info_t *info, bool anim)
{
    const int8_t dir = info->mode.select_slide_dir;
    uint8_t old_center = info->mode.new_index;

    pump_home_mode_select_carousel_slide_stop(info);
    pump_home_mode_select_carousel_visible_set(info, true);

    if (dir == 1) {
        old_center = pump_home_mode_neighbor_prev_get(info, info->mode.new_index);
    } else if (dir == -1) {
        old_center = pump_home_mode_neighbor_next_get(info, info->mode.new_index);
    }

    if (!anim || info->force_update || dir == 0) {
        pump_home_mode_select_carousel_content_update(info);
        return;
    }

    pump_home_mode_select_carousel_content_update_for_center(info, old_center);
    const lv_coord_t step = pump_home_mode_select_carousel_step_get(info, dir);
    if (step == 0) {
        pump_home_mode_select_carousel_content_update(info);
        return;
    }
    const int32_t target = (int32_t)dir * step;

    lv_anim_t slide_anim;
    lv_anim_init(&slide_anim);
    lv_anim_set_var(&slide_anim, info);
    lv_anim_set_user_data(&slide_anim, info);
    lv_anim_set_time(&slide_anim, PUMP_HOME_SELECT_ANIM_TIME_MS);
    lv_anim_set_path_cb(&slide_anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&slide_anim, pump_home_mode_select_carousel_slide_exec);
    lv_anim_set_values(&slide_anim, 0, target);
    lv_anim_set_ready_cb(&slide_anim, pump_home_mode_select_carousel_slide_ready_cb);
    lv_anim_start(&slide_anim);
}

static void pump_home_mode_anim_exec_opa(void *var, int32_t v);
static void pump_home_mode_obj_start_opa_anim(lv_obj_t *obj, lv_opa_t from, lv_opa_t to, bool show);

static void pump_home_mode_select_carousel_enter(pump_home_base_ui_info_t *info, bool anim)
{
    pump_home_mode_select_carousel_slide_stop(info);
    pump_home_mode_select_carousel_content_update(info);
    pump_home_mode_select_carousel_visible_set(info, true);

    if (!anim || info->force_update) {
        for (uint8_t i = 0; i < 3; i++) {
            if (info->mode.select_titles[i] != NULL && lv_obj_is_valid(info->mode.select_titles[i])) {
                lv_obj_set_style_opa(info->mode.select_titles[i], LV_OPA_COVER, 0);
            }
        }
        return;
    }

    for (uint8_t i = 0; i < 3; i++) {
        if (info->mode.select_titles[i] == NULL || !lv_obj_is_valid(info->mode.select_titles[i])) {
            continue;
        }
        lv_anim_del(info->mode.select_titles[i], pump_home_mode_anim_exec_opa);
        lv_obj_set_style_opa(info->mode.select_titles[i], LV_OPA_TRANSP, 0);
        pump_home_mode_obj_start_opa_anim(info->mode.select_titles[i], LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
}

static void pump_home_mode_select_carousel_exit(pump_home_base_ui_info_t *info)
{
    pump_home_mode_select_carousel_slide_stop(info);
    pump_home_mode_select_carousel_visible_set(info, false);
}

static void pump_home_mode_select_carousel_refresh(pump_home_base_ui_info_t *info, bool mode_index_changed,
    bool anim)
{
    if (mode_index_changed) {
        pump_home_mode_select_carousel_slide(info, anim);
        return;
    }
    pump_home_mode_select_carousel_content_update(info);
    pump_home_mode_select_carousel_visible_set(info, true);
}
#endif

static void pump_home_mode_anim_exec_opa(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
}

static void pump_home_mode_anim_exec_y(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_set_y(obj, (lv_coord_t)v);
}

static void pump_home_mode_obj_start_opa_anim(lv_obj_t *obj, lv_opa_t from, lv_opa_t to, bool show)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }

    if (show) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
    lv_anim_del(obj, pump_home_mode_anim_exec_opa);
    lv_obj_set_style_opa(obj, from, 0);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_time(&anim, PUMP_HOME_SELECT_ANIM_TIME_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, pump_home_mode_anim_exec_opa);
    lv_anim_set_values(&anim, from, to);
    lv_anim_start(&anim);
}

static void pump_home_mode_select_wave_offset_anim_exec(void *var, int32_t v)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)var;
    info->mode.select_wave_y_offset = (int16_t)v;
    pump_home_base_select_offset_apply(info);
}

static void pump_home_mode_select_clock_offset_anim_exec(void *var, int32_t v)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)var;
    info->mode.select_clock_y_offset = (int16_t)v;
    pump_home_base_select_offset_apply(info);
}

static void pump_home_mode_select_clock_2seg_offset_anim_exec(void *var, int32_t v)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)var;
    info->mode.select_clock_2seg_y_offset = (int16_t)v;
    pump_home_base_select_offset_apply(info);
}

static void pump_home_mode_select_mode_offset_anim_exec(void *var, int32_t v)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)var;
    info->mode.select_mode_y_offset = (int16_t)v;
    pump_home_base_select_offset_apply(info);
}

static void pump_home_mode_select_remain_offset_anim_exec(void *var, int32_t v)
{
    pump_home_base_ui_info_t *info = (pump_home_base_ui_info_t *)var;
    info->mode.select_remain_y_offset = (int16_t)v;
    pump_home_base_select_offset_apply(info);
}

static void pump_home_mode_obj_start_offset_anim(pump_home_base_ui_info_t *info,
    lv_anim_exec_xcb_t exec_cb, int32_t from, int32_t to)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, info);
    lv_anim_set_time(&anim, PUMP_HOME_SELECT_ANIM_TIME_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, exec_cb);
    lv_anim_set_values(&anim, from, to);
    lv_anim_start(&anim);
}

static void pump_home_mode_select_layout_shift(pump_home_base_ui_info_t *info, bool shift_down, bool anim)
{
    int16_t target_wave = 0;
    int16_t target_clock = 0;
    int16_t target_clock_2seg = 0;
    int16_t target_mode = 0;
    int16_t target_remain = 0;

    pump_home_mode_select_layout_target_get(info, shift_down, &target_wave, &target_clock,
        &target_clock_2seg, &target_mode, &target_remain);

    if (!anim || info->force_update) {
        info->mode.select_wave_y_offset = target_wave;
        info->mode.select_clock_y_offset = target_clock;
        info->mode.select_clock_2seg_y_offset = target_clock_2seg;
        info->mode.select_mode_y_offset = target_mode;
        info->mode.select_remain_y_offset = target_remain;
        pump_home_base_select_offset_apply(info);
        return;
    }

    lv_anim_del(info, pump_home_mode_select_wave_offset_anim_exec);
    lv_anim_del(info, pump_home_mode_select_clock_offset_anim_exec);
    lv_anim_del(info, pump_home_mode_select_clock_2seg_offset_anim_exec);
    lv_anim_del(info, pump_home_mode_select_mode_offset_anim_exec);
    lv_anim_del(info, pump_home_mode_select_remain_offset_anim_exec);
    pump_home_mode_obj_start_offset_anim(info, pump_home_mode_select_wave_offset_anim_exec,
        info->mode.select_wave_y_offset, target_wave);
    pump_home_mode_obj_start_offset_anim(info, pump_home_mode_select_clock_offset_anim_exec,
        info->mode.select_clock_y_offset, target_clock);
    pump_home_mode_obj_start_offset_anim(info, pump_home_mode_select_clock_2seg_offset_anim_exec,
        info->mode.select_clock_2seg_y_offset, target_clock_2seg);
    if (info->mode.select_mode_y_offset != target_mode) {
        pump_home_mode_obj_start_offset_anim(info, pump_home_mode_select_mode_offset_anim_exec,
            info->mode.select_mode_y_offset, target_mode);
    }
    if (info->mode.select_remain_y_offset != target_remain) {
        pump_home_mode_obj_start_offset_anim(info, pump_home_mode_select_remain_offset_anim_exec,
            info->mode.select_remain_y_offset, target_remain);
    }
}

#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
static void pump_home_mode_capsule_create(pump_home_base_ui_info_t *info)
{
    bool capsule_newly_created = false;

    if (info->mode.title_capsule == NULL || !lv_obj_is_valid(info->mode.title_capsule)) {
        info->mode.title_capsule = lv_obj_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.title_capsule);
        lv_obj_remove_style_all(info->mode.title_capsule);
        lv_obj_set_style_radius(info->mode.title_capsule, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(info->mode.title_capsule, lv_color_hex(PUMP_HOME_CAPSULE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(info->mode.title_capsule, LV_OPA_COVER, 0);
        lv_obj_clear_flag(info->mode.title_capsule, LV_OBJ_FLAG_SCROLLABLE);
        capsule_newly_created = true;
    }

    if (info->mode.capsule_title == NULL || !lv_obj_is_valid(info->mode.capsule_title)) {
        info->mode.capsule_title = lv_label_create(info->mode.title_capsule);
        EXCEPTION_CHECK_VOID(info->mode.capsule_title);
        lv_obj_set_style_text_color(info->mode.capsule_title, lv_color_hex(PUMP_HOME_CAPSULE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.capsule_title, &lv_font_aeonik_soft_20_bold, 0);
        lv_obj_set_style_text_align(info->mode.capsule_title, LV_TEXT_ALIGN_CENTER, 0);
    }

    if (capsule_newly_created) {
        lv_obj_add_flag(info->mode.title_capsule, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(info->mode.title_capsule, LV_OPA_TRANSP, 0);
    }
}

static void pump_home_mode_dots_capsule_size_get(const pump_home_base_ui_info_t *info,
    lv_coord_t *capsule_width, lv_coord_t *capsule_height)
{
    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        *capsule_height = PUMP_HOME_BF_CAPSULE_H;
        if (info->mode.capsule_title != NULL && lv_obj_is_valid(info->mode.capsule_title)) {
            lv_obj_update_layout(info->mode.capsule_title);
            *capsule_width = lv_obj_get_width(info->mode.capsule_title) + PUMP_HOME_BF_CAPSULE_W_PAD;
        } else {
            *capsule_width = PUMP_HOME_CAPSULE_W;
        }
        return;
    }
    *capsule_width = PUMP_HOME_CAPSULE_W;
    *capsule_height = PUMP_HOME_CAPSULE_H;
}

static void pump_home_mode_dots_create(pump_home_base_ui_info_t *info)
{
    lv_obj_t *capsule = info->mode.title_capsule;
    if (capsule == NULL || !lv_obj_is_valid(capsule)) {
        return;
    }

    uint8_t dot_count = info->mode.select_dot_count;
    if (dot_count == 0) {
        dot_count = 1;
    }

    lv_coord_t capsule_width = 0;
    lv_coord_t capsule_height = 0;
    pump_home_mode_dots_capsule_size_get(info, &capsule_width, &capsule_height);

    lv_coord_t total_width = dot_count * PUMP_HOME_DOT_SIZE + (dot_count - 1) * PUMP_HOME_DOT_GAP;
    lv_coord_t start_x = (capsule_width - total_width) / 2;
    lv_coord_t dot_y = capsule_height - PUMP_HOME_CAPSULE_DOT_BOTTOM_PAD - PUMP_HOME_DOT_SIZE;

    for (uint8_t i = 0; i < PUMP_HOME_SELECT_DOT_MAX; i++) {
        if (info->mode.select_dots[i] != NULL && lv_obj_is_valid(info->mode.select_dots[i]) &&
            lv_obj_get_parent(info->mode.select_dots[i]) != capsule) {
            lv_obj_del(info->mode.select_dots[i]);
            info->mode.select_dots[i] = NULL;
        }
        if (info->mode.select_dots[i] == NULL || !lv_obj_is_valid(info->mode.select_dots[i])) {
            info->mode.select_dots[i] = lv_obj_create(capsule);
            EXCEPTION_CHECK_VOID(info->mode.select_dots[i]);
            lv_obj_remove_style_all(info->mode.select_dots[i]);
            lv_obj_set_size(info->mode.select_dots[i], PUMP_HOME_DOT_SIZE, PUMP_HOME_DOT_SIZE);
            lv_obj_set_style_radius(info->mode.select_dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(info->mode.select_dots[i], lv_color_hex(PUMP_HOME_DOT_ACTIVE_COLOR), 0);
            lv_obj_set_style_bg_opa(info->mode.select_dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(info->mode.select_dots[i], 0, 0);
            lv_obj_clear_flag(info->mode.select_dots[i], LV_OBJ_FLAG_SCROLLABLE);
        }
        if (i >= dot_count) {
            pump_home_mode_obj_visible_set(info->mode.select_dots[i], false);
            continue;
        }
        lv_obj_clear_flag(info->mode.select_dots[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(info->mode.select_dots[i], LV_ALIGN_TOP_LEFT,
            start_x + i * (PUMP_HOME_DOT_SIZE + PUMP_HOME_DOT_GAP), dot_y);
    }
}

static void pump_home_mode_dots_update(pump_home_base_ui_info_t *info, uint8_t active_idx)
{
    pump_home_mode_dots_create(info);
    if (active_idx >= info->mode.select_dot_count) {
        active_idx = 0;
    }

    for (uint8_t i = 0; i < PUMP_HOME_SELECT_DOT_MAX; i++) {
        lv_obj_t *dot = info->mode.select_dots[i];
        if (dot == NULL || !lv_obj_is_valid(dot)) {
            continue;
        }
        if (i >= info->mode.select_dot_count) {
            pump_home_mode_obj_visible_set(dot, false);
            continue;
        }
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(dot, lv_color_hex(i == active_idx ?
            PUMP_HOME_DOT_ACTIVE_COLOR : PUMP_HOME_DOT_INACTIVE_COLOR), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    }
}

static void pump_home_mode_capsule_common_layout(pump_home_base_ui_info_t *info, const char *title)
{
    pump_home_mode_capsule_create(info);
    if (info->mode.title_capsule == NULL || info->mode.capsule_title == NULL) {
        return;
    }

    lv_label_set_text(info->mode.capsule_title, title);
    lv_obj_set_style_text_font(info->mode.capsule_title, &lv_font_aeonik_soft_20_bold, 0);
    lv_obj_set_style_text_align(info->mode.capsule_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(info->mode.title_capsule, PUMP_HOME_CAPSULE_W, PUMP_HOME_CAPSULE_H);
    lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0, PUMP_HOME_CAPSULE_Y);
    lv_obj_align(info->mode.capsule_title, LV_ALIGN_TOP_MID, 0, PUMP_HOME_COMMON_CAPSULE_TITLE_Y);
    pump_home_mode_dots_create(info);
}

static void pump_home_mode_capsule_bf_layout(pump_home_base_ui_info_t *info, const char *title)
{
    lv_coord_t capsule_width = 0;
    lv_coord_t capsule_height = 0;

    pump_home_mode_capsule_create(info);
    if (info->mode.title_capsule == NULL || info->mode.capsule_title == NULL) {
        return;
    }

    lv_label_set_text(info->mode.capsule_title, title);
    lv_obj_set_style_text_font(info->mode.capsule_title, &lv_font_aeonik_soft_20_bold, 0);
    lv_obj_set_style_text_align(info->mode.capsule_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_update_layout(info->mode.capsule_title);
    capsule_width = lv_obj_get_width(info->mode.capsule_title) + PUMP_HOME_BF_CAPSULE_W_PAD;
    capsule_height = PUMP_HOME_BF_CAPSULE_H;
    lv_obj_set_size(info->mode.title_capsule, capsule_width, capsule_height);
    lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0, PUMP_HOME_BF_CAPSULE_Y);
    lv_obj_align(info->mode.capsule_title, LV_ALIGN_TOP_MID, 0, PUMP_HOME_BF_CAPSULE_TITLE_Y);
    pump_home_mode_dots_create(info);
}

static void pump_home_mode_capsule_layout(pump_home_base_ui_info_t *info, uint8_t mode)
{
    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        pump_home_mode_capsule_bf_layout(info, pump_home_mode_bf_name_get(info, mode));
        return;
    }
    pump_home_mode_capsule_common_layout(info, pump_home_mode_common_name_get(mode));
}
#endif

static void pump_home_mode_default_title_restore(pump_home_base_ui_info_t *info)
{
    char rhythm_name[24] = {0};

    switch (info->mode.index) {
        case BREAST_PUMP_STIMULATION_MODE:
            pump_home_mode_title_update(info, "Stimulation");
            pump_home_mode_name_update(info, NULL, 0, 0);
            break;
        case BREAST_PUMP_LACTATION_MODE:
            pump_home_mode_title_update(info, "Expression");
            pump_home_mode_name_update(info, NULL, 0, 0);
            break;
        case BREAST_PUMP_MIX_MODE:
            pump_home_mode_title_update(info, "Mixed");
            pump_home_mode_name_update(info, NULL, 0, 0);
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            pump_home_mode_title_update(info, "Milk Boost");
            pump_home_mode_name_update(info,
                pump_home_mode_type_name_get(info->mode.mode_type),
                PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            pump_home_mode_title_update(info, "Power Pumping");
            pump_home_mode_name_update(info,
                pump_home_mode_type_name_get(info->mode.mode_type),
                PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            snprintf(rhythm_name, sizeof(rhythm_name), "Customize %s", info->mode.custom_id_str);
            pump_home_mode_title_update(info, rhythm_name);
            pump_home_mode_name_update(info,
                pump_home_mode_type_name_get(info->mode.mode_type),
                PUMP_HOME_MODE_NAME_X, PUMP_HOME_MODE_NAME_Y);
            break;
        default:
            break;
    }
}

static void pump_home_mode_select_state_sync(pump_home_base_ui_info_t *info,
    pump_mode_display_type_e display_type)
{
    info->mode.select_capsule_style = pump_home_mode_select_capsule_style_get(display_type,
        info->mode.new_index);
    info->mode.custom_mode_active = pump_home_mode_custom_active_get(info) ? 1 : 0;
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
    if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_BF) {
        info->mode.select_dot_count = pump_home_mode_bf_dot_count_get(info->mode.custom_mode_active != 0);
    } else if (info->mode.select_capsule_style == PUMP_HOME_CAPSULE_STYLE_COMMON) {
        info->mode.select_dot_count = PUMP_HOME_SELECT_DOT_MAX;
    } else {
        info->mode.select_dot_count = 0;
    }
#endif
}

static void pump_home_mode_select_default_hide(pump_home_base_ui_info_t *info, bool anim)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        return;
    }
    if (!anim || info->force_update) {
        pump_home_mode_obj_visible_set(info->mode.title, false);
        return;
    }
    pump_home_mode_obj_start_opa_anim(info->mode.title, LV_OPA_COVER, LV_OPA_TRANSP, false);
}

#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
static void pump_home_mode_select_group_visible_set(pump_home_base_ui_info_t *info, bool visible,
    lv_opa_t opa)
{
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        pump_home_mode_obj_visible_set(info->mode.title_capsule, visible);
        lv_obj_set_style_opa(info->mode.title_capsule, opa, 0);
    }
}
#endif

#if PUMP_HOME_MODE_SELECT_CAROUSEL_ENABLE
static void pump_home_mode_select_carousel_default_cut(pump_home_base_ui_info_t *info)
{
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_anim_del(info->mode.title, NULL);
        lv_obj_set_style_opa(info->mode.title, LV_OPA_COVER, 0);
        pump_home_mode_obj_visible_set(info->mode.title, false);
    }
}
#endif

static void pump_home_mode_select_enter(pump_home_base_ui_info_t *info, bool anim)
{
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
    lv_coord_t capsule_y = pump_home_mode_select_capsule_y_get(info);
    uint8_t dot_idx = pump_home_mode_select_dot_index_get(info, info->mode.new_index);

    pump_home_mode_capsule_layout(info, info->mode.new_index);
    pump_home_mode_dots_update(info, dot_idx);
    pump_home_mode_select_layout_shift(info, true, anim);
    pump_home_mode_select_default_hide(info, anim);

    if (!anim || info->force_update) {
        pump_home_mode_select_group_visible_set(info, true, LV_OPA_COVER);
        if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
            lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0, capsule_y);
        }
        return;
    }

    pump_home_mode_select_group_visible_set(info, true, LV_OPA_TRANSP);
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0,
            capsule_y - PUMP_HOME_SELECT_ENTER_Y_OFFSET);
        lv_anim_del(info->mode.title_capsule, pump_home_mode_anim_exec_y);
        lv_anim_del(info->mode.title_capsule, pump_home_mode_anim_exec_opa);

        lv_anim_t anim_y;
        lv_anim_init(&anim_y);
        lv_anim_set_var(&anim_y, info->mode.title_capsule);
        lv_anim_set_time(&anim_y, PUMP_HOME_SELECT_ANIM_TIME_MS);
        lv_anim_set_path_cb(&anim_y, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&anim_y, pump_home_mode_anim_exec_y);
        lv_anim_set_values(&anim_y, capsule_y - PUMP_HOME_SELECT_ENTER_Y_OFFSET, capsule_y);
        lv_anim_start(&anim_y);

        pump_home_mode_obj_start_opa_anim(info->mode.title_capsule, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
#else
    pump_home_mode_select_carousel_default_cut(info);
    pump_home_mode_select_carousel_enter(info, anim);
    pump_home_mode_select_layout_shift(info, true, anim);
#endif
}

static void pump_home_mode_select_exit(pump_home_base_ui_info_t *info, bool anim)
{
    pump_home_mode_select_layout_shift(info, false, anim);

#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
    if (!anim || info->force_update) {
        pump_home_mode_select_group_visible_set(info, false, LV_OPA_TRANSP);
        pump_home_mode_default_title_restore(info);
        pump_home_base_select_offset_apply(info);
        return;
    }

    pump_home_mode_default_title_restore(info);
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        pump_home_mode_obj_start_opa_anim(info->mode.title, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        pump_home_mode_obj_start_opa_anim(info->mode.title_capsule, LV_OPA_COVER, LV_OPA_TRANSP, false);
    }
    pump_home_base_select_offset_apply(info);
#else
    pump_home_mode_select_carousel_exit(info);

    if (!anim || info->force_update) {
        pump_home_mode_default_title_restore(info);
        pump_home_base_select_offset_apply(info);
        return;
    }

    pump_home_mode_default_title_restore(info);
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        pump_home_mode_obj_start_opa_anim(info->mode.title, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
    pump_home_base_select_offset_apply(info);
#endif
}

static void pump_home_mode_select_refresh(pump_home_base_ui_info_t *info, bool mode_index_changed, bool anim)
{
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
    uint8_t dot_idx = pump_home_mode_select_dot_index_get(info, info->mode.new_index);

    pump_home_mode_capsule_layout(info, info->mode.new_index);
    pump_home_mode_dots_update(info, dot_idx);
    pump_home_mode_obj_visible_set(info->mode.title, false);
    pump_home_mode_select_group_visible_set(info, true, LV_OPA_COVER);
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0,
            pump_home_mode_select_capsule_y_get(info));
    }
#else
    pump_home_mode_obj_visible_set(info->mode.title, false);
    pump_home_mode_select_carousel_refresh(info, mode_index_changed, anim);
#endif
}

static void pump_home_mode_select_ui_apply(pump_home_base_ui_info_t *info,
    pump_mode_display_type_e display_type, bool mode_index_changed)
{
    const bool select_ui = pump_home_mode_select_ui_active(display_type, info->mode.new_index);
    const bool prev_select_ui = info->mode.select_ui_active != 0;
    const bool anim = !info->force_update;
    const uint8_t prev_style = info->mode.select_capsule_style;
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
    const uint8_t prev_dot_count = info->mode.select_dot_count;
#endif
    const bool prev_custom_active = info->mode.custom_mode_active;

    pump_home_mode_select_state_sync(info, display_type);

    if (select_ui && !prev_select_ui) {
        pump_home_mode_select_enter(info, anim);
    } else if (!select_ui && prev_select_ui) {
        pump_home_mode_select_exit(info, anim);
    } else if (select_ui) {
        const bool style_changed = prev_style != info->mode.select_capsule_style;
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
        const bool dot_count_changed = prev_dot_count != info->mode.select_dot_count;
#else
        const bool dot_count_changed = false;
#endif
        const bool custom_changed = prev_custom_active != info->mode.custom_mode_active;

        if (style_changed || dot_count_changed || custom_changed || mode_index_changed) {
            pump_home_mode_select_refresh(info, mode_index_changed, anim);
            if (style_changed || mode_index_changed) {
                pump_home_mode_select_layout_shift(info, true, anim);
            }
        }
    } else if (!select_ui && (prev_select_ui || info->force_update)) {
        if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
            pump_home_mode_obj_visible_set(info->mode.title, true);
            lv_obj_set_style_opa(info->mode.title, LV_OPA_COVER, 0);
        }
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
        pump_home_mode_select_group_visible_set(info, false, LV_OPA_TRANSP);
#else
        pump_home_mode_select_carousel_exit(info);
#endif
    }

    info->mode.select_ui_active = select_ui ? 1 : 0;
    info->mode.display_type = (uint8_t)display_type;
}
#endif

void pump_home_base_select_offset_apply(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);

    if (info->animation.wave != NULL && lv_obj_is_valid(info->animation.wave) &&
        !lv_obj_has_flag(info->animation.wave, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_align(info->animation.wave, LV_ALIGN_TOP_MID, 0,
            PUMP_HOME_WAVE_Y + info->mode.select_wave_y_offset);
    }

    if (info->mode.mode != NULL && lv_obj_is_valid(info->mode.mode) &&
        !lv_obj_has_flag(info->mode.mode, LV_OBJ_FLAG_HIDDEN)) {
        if (info->mode.mode_name_x == 0) {
            lv_obj_align(info->mode.mode, LV_ALIGN_TOP_MID, 0,
                info->mode.mode_name_y + info->mode.select_mode_y_offset);
        } else {
            lv_obj_align(info->mode.mode, LV_ALIGN_TOP_LEFT, info->mode.mode_name_x,
                info->mode.mode_name_y + info->mode.select_mode_y_offset);
        }
    }

    pump_home_base_time_apply_select_offset(info);
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_mode_ui_remove(pump_home_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
#if PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE
    lv_anim_del(info, NULL);
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_anim_del(info->mode.title, NULL);
    }
    if (info->mode.mode != NULL && lv_obj_is_valid(info->mode.mode)) {
        lv_anim_del(info->mode.mode, NULL);
    }
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        lv_anim_del(info->mode.title_capsule, NULL);
    }
#else
    info->mode.select_title_slide_offset = 0;
    info->mode.select_slide_dir = 0;
#endif
#endif
}

void pump_home_base_mode_ui_update(pump_home_base_ui_info_t *info, bool blink_visible,
    pump_mode_display_type_e display_type)
{
    EXCEPTION_CHECK_VOID(info);

    const bool mode_index_changed = info->force_update ||
        info->mode.index != info->mode.new_index;
    const bool mode_type_changed = info->mode.mode_type != info->mode.new_mode_type;
#if PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE
    const bool select_ui = pump_home_mode_select_ui_active(display_type, info->mode.new_index);
    const bool select_ui_transition = (info->mode.select_ui_active != 0) != select_ui;
    const bool display_changed = info->mode.display_type != (uint8_t)display_type;
#endif

#if PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE
    if (!info->force_update && !mode_index_changed && !mode_type_changed &&
        !select_ui_transition && !display_changed) {
#else
    if (!info->force_update && !mode_index_changed && !mode_type_changed) {
#endif
#if PUMP_HOME_MODE_TITLE_BLINK_ENABLE
        pump_home_mode_title_blink_apply(info, blink_visible);
#else
        (void)blink_visible;
#endif
        return;
    }

    if (mode_index_changed) {
#if PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE && !PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
        if (info->mode.select_ui_active) {
            info->mode.select_slide_dir = pump_home_mode_select_slide_dir_get(info);
        } else {
            info->mode.select_slide_dir = 0;
        }
#endif
#if PUMP_HOME_MODE_SELECT_CAPSULE_ENABLE
        pump_home_mode_capsule_create(info);
#endif
        pump_home_mode_apply_index_change(info);
    } else if (mode_type_changed) {
        pump_home_mode_apply_type_change(info);
    }

#if PUMP_HOME_MODE_SELECT_TRANSITION_ENABLE
    pump_home_mode_select_ui_apply(info, display_type, mode_index_changed);
#else
    (void)display_type;
#endif

#if PUMP_HOME_MODE_TITLE_BLINK_ENABLE
    pump_home_mode_title_blink_apply(info, blink_visible);
#else
    (void)blink_visible;
#endif
}
