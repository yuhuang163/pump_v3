/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "home_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

#if IS_SUPPORT_HOME_CUSTOM_MODE_CMP
#define HOME_MODE_TITLE_COLOR                      (0x00FF00)
#define HOME_MODE_CMP_INDEX                        BREAST_PUMP_MILK_BOOST_MODE
#else
#define HOME_MODE_TITLE_COLOR                      (0xBD4060)
#endif

#define HOME_MODE_TITLE_Y                          (72)
#define HOME_MODE_ICON_WIDTH                       (32)
#define HOME_MODE_ICON_TITLE_GAP                   (4)
#define HOME_MODE_ICON_Y                           (66)
#define HOME_MODE_ICON_TITLE_Y_GAP                 (HOME_MODE_TITLE_Y - HOME_MODE_ICON_Y)

#define HOME_MODE_SELECT_DOT_MAX                   (3)
#define HOME_MODE_CAPSULE_Y                        (64)
#define HOME_MODE_CAPSULE_W                        (142)
#define HOME_MODE_CAPSULE_H                        (50)
#define HOME_MODE_COMMON_CAPSULE_TITLE_Y           (10)

#define HOME_MODE_BF_CAPSULE_Y                     (66)
#define HOME_MODE_BF_CAPSULE_W                     (220)
#define HOME_MODE_BF_CAPSULE_H                     (54)
#define HOME_MODE_BF_CAPSULE_ICON_Y                (12)

#define HOME_MODE_CAPSULE_DOT_BOTTOM_PAD           (8)
#define HOME_MODE_CAPSULE_BG_COLOR                 (0xBD4060)
#define HOME_MODE_CAPSULE_TITLE_COLOR              (0xFFFFFF)

#define HOME_MODE_DOT_SIZE                         (6)
#define HOME_MODE_DOT_GAP                          (8)
#define HOME_MODE_DOT_ACTIVE_COLOR                 (0xFFFFFF)
#define HOME_MODE_DOT_INACTIVE_COLOR               (0xD58599)

#define HOME_MODE_SELECT_ANIM_TIME_MS              (300)
#define HOME_MODE_SELECT_ENTER_Y_OFFSET            (8)

/* 选择态布局偏移：最终 Y = 各控件基准 Y + 下列 offset */
#if HOME_MODE_SELECT_CAPSULE_ENABLE
/* COMMON 胶囊（Stimulation / Expression / Mixed） */
#define HOME_MODE_SELECT_COMMON_STIM_LABEL_Y_OFFSET  (12)
#define HOME_MODE_SELECT_COMMON_EXPR_LABEL_Y_OFFSET  (12)
#define HOME_MODE_SELECT_COMMON_MIX_LABEL_Y_OFFSET   (16)
#define HOME_MODE_SELECT_COMMON_TIME_Y_OFFSET        (0)
/* BF 胶囊（Milk Boost / Power Pumping / Custom） */
#define HOME_MODE_SELECT_BF_MB_LABEL_Y_OFFSET        (18)
#define HOME_MODE_SELECT_BF_MB_TIME_Y_OFFSET         (26)
#define HOME_MODE_SELECT_BF_PP_LABEL_Y_OFFSET        (22)
#define HOME_MODE_SELECT_BF_PP_TIME_Y_OFFSET         (26)
#define HOME_MODE_SELECT_BF_CUSTOM_LABEL_Y_OFFSET    (22)
#define HOME_MODE_SELECT_BF_CUSTOM_TIME_Y_OFFSET     (26)
#elif HOME_MODE_SELECT_CAROUSEL_ENABLE
#define HOME_MODE_SELECT_STIM_LABEL_Y_OFFSET         (12)
#define HOME_MODE_SELECT_STIM_TIME_Y_OFFSET          (0)

#define HOME_MODE_SELECT_EXPR_LABEL_Y_OFFSET         (12)
#define HOME_MODE_SELECT_EXPR_TIME_Y_OFFSET          (0)

#define HOME_MODE_SELECT_MIX_LABEL_Y_OFFSET          (16)
#define HOME_MODE_SELECT_MIX_TIME_Y_OFFSET           (0)

#define HOME_MODE_SELECT_MB_LABEL_Y_OFFSET           (16)
#define HOME_MODE_SELECT_MB_TIME_Y_OFFSET            (16)

#define HOME_MODE_SELECT_PP_LABEL_Y_OFFSET           (16)
#define HOME_MODE_SELECT_PP_TIME_Y_OFFSET            (16)

#define HOME_MODE_SELECT_CUSTOM_LABEL_Y_OFFSET       (16)
#define HOME_MODE_SELECT_CUSTOM_TIME_Y_OFFSET        (16)
#endif

#define HOME_MODE_SELECT_TITLE_ACTIVE_Y              (HOME_MODE_TITLE_Y)
#define HOME_MODE_SELECT_TITLE_INACTIVE_Y            (HOME_MODE_TITLE_Y + 2)
#define HOME_MODE_SELECT_TITLE_GAP                   24
#define HOME_MODE_SELECT_CAROUSEL_SIDE_COLOR         (0xE89AAB)
#define HOME_MODE_SELECT_SLOT_PREV                   0
#define HOME_MODE_SELECT_SLOT_CUR                    1
#define HOME_MODE_SELECT_SLOT_NEXT                   2

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void home_pump_mode_title_update(home_custom_ui_info_t *info, const char *title,
    uint16_t icon_resource_id)
{
    if (info->mode.title == NULL || !lv_obj_is_valid(info->mode.title)) {
        info->mode.title = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.title);
        lv_obj_set_style_text_color(info->mode.title, lv_color_hex(HOME_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.title, &lv_font_aeonik_soft_24_bold, 0);
    }

    if (title == NULL) {
        lv_obj_add_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon)) {
            ui_img_baj_release_decoded_buf(info->mode.icon);
            lv_obj_add_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(info->mode.title, title);

    if (icon_resource_id == 0) {
        if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon)) {
            ui_img_baj_release_decoded_buf(info->mode.icon);
            lv_obj_add_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_align(info->mode.title, LV_ALIGN_TOP_MID, 0, HOME_MODE_TITLE_Y);
        return;
    }

    if (info->mode.icon == NULL || !lv_obj_is_valid(info->mode.icon)) {
        info->mode.icon = lv_image_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.icon);
    }
    lv_obj_clear_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->mode.icon, icon_resource_id,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);

    lv_obj_update_layout(info->mode.title);
    lv_coord_t title_width = lv_obj_get_width(info->mode.title);
    lv_coord_t screen_width = lv_obj_get_width(info->root);
    lv_coord_t total_width = HOME_MODE_ICON_WIDTH + HOME_MODE_ICON_TITLE_GAP + title_width;
    lv_coord_t icon_x = (screen_width - total_width) / 2;
    lv_coord_t title_x = icon_x + HOME_MODE_ICON_WIDTH + HOME_MODE_ICON_TITLE_GAP;

    lv_obj_align(info->mode.icon, LV_ALIGN_TOP_LEFT, icon_x, HOME_MODE_ICON_Y);
    lv_obj_align(info->mode.title, LV_ALIGN_TOP_LEFT, title_x, HOME_MODE_TITLE_Y);
}

static void home_pump_mode_bf_capsule_icon_title_layout(lv_obj_t *capsule, lv_obj_t *icon,
    lv_obj_t *title, lv_coord_t capsule_width)
{
    if (capsule == NULL || icon == NULL || title == NULL) {
        return;
    }

    lv_obj_update_layout(icon);
    lv_obj_update_layout(title);
    lv_coord_t icon_width = lv_obj_get_width(icon);
    if (icon_width <= 0) {
        icon_width = HOME_MODE_ICON_WIDTH;
    }
    lv_coord_t title_width = lv_obj_get_width(title);
    lv_coord_t total_width = icon_width + HOME_MODE_ICON_TITLE_GAP + title_width;
    lv_coord_t icon_x = (capsule_width - total_width) / 2;
    lv_coord_t title_x = icon_x + icon_width + HOME_MODE_ICON_TITLE_GAP;
    lv_coord_t icon_y = HOME_MODE_BF_CAPSULE_ICON_Y;
    lv_coord_t title_y = icon_y + HOME_MODE_ICON_TITLE_Y_GAP;

    lv_obj_align(icon, LV_ALIGN_TOP_LEFT, icon_x, icon_y);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, title_x, title_y);
}

static void home_pump_mode_obj_visible_set(lv_obj_t *obj, bool visible)
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

#if HOME_MODE_TITLE_BLINK_ENABLE
static void home_pump_mode_title_blink_apply(home_custom_ui_info_t *info, bool blink_visible)
{
    const bool visible = (info->mode.mode_blink_state == 0) || blink_visible;
    home_pump_mode_obj_visible_set(info->mode.title, visible);
}

static void home_pump_mode_icon_blink_apply(home_custom_ui_info_t *info, bool blink_visible)
{
    if (info->mode.icon == NULL || !lv_obj_is_valid(info->mode.icon)) {
        return;
    }
    const bool visible = (info->mode.icon_blink_state == 0) || blink_visible;
    home_pump_mode_obj_visible_set(info->mode.icon, visible);
}
#endif

static void home_pump_mode_apply_index_change(home_custom_ui_info_t *info)
{
    switch (info->mode.new_index) {
        default:
        case BREAST_PUMP_STIMULATION_MODE:
            home_pump_mode_title_update(info, "Stimulation", 0);
            break;
        case BREAST_PUMP_LACTATION_MODE:
            home_pump_mode_title_update(info, "Expression", 0);
            break;
        case BREAST_PUMP_MIX_MODE:
            home_pump_mode_title_update(info, "Mixed", 0);
            break;
        case BREAST_PUMP_CUSTOM_MODE: {
            char custom_str[24] = {0};
            if (strlen(info->mode.custom_id_str) > 0) {
                sprintf(custom_str, "Customized %s", info->mode.custom_id_str);
            } else {
                sprintf(custom_str, "Customized 01");
            }
            home_pump_mode_title_update(info, custom_str, CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
            break;
        }
        case BREAST_PUMP_MILK_BOOST_MODE:
            home_pump_mode_title_update(info, "Milk Boost", CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            home_pump_mode_title_update(info, "Power Pumping", CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
            break;
    }

    info->mode.index = info->mode.new_index;
}

#if HOME_MODE_SELECT_TRANSITION_ENABLE
static bool home_pump_mode_is_bf_select_mode(uint8_t mode)
{
    return mode == BREAST_PUMP_MILK_BOOST_MODE ||
        mode == BREAST_PUMP_PP_MILK_TRACK_MODE ||
        mode == BREAST_PUMP_CUSTOM_MODE;
}

static bool home_pump_mode_custom_active_get(home_custom_ui_info_t *info)
{
    int activate_status = 0;

    if (info->ui == NULL) {
        return false;
    }
    ui_proc_home_t *proc = (ui_proc_home_t *)info->ui->priv.play_info.user_data;
    if (proc == NULL || proc->pump == NULL) {
        return false;
    }
    READ_PUMP_MODE_ACTIVATE(proc->pump, BREAST_PUMP_CUSTOM_MODE, activate_status);
    return activate_status > 0;
}

static const char *home_pump_mode_common_name_get(uint8_t mode)
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

static const char *home_pump_mode_bf_name_get(home_custom_ui_info_t *info, uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_MILK_BOOST_MODE:
            return "Milk Boost";
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return "Power Pumping";
        case BREAST_PUMP_CUSTOM_MODE: {
            static char custom_title[24];
            if (strlen(info->mode.custom_id_str) > 0) {
                snprintf(custom_title, sizeof(custom_title), "Customized %s", info->mode.custom_id_str);
            } else {
                snprintf(custom_title, sizeof(custom_title), "Customized 01");
            }
            return custom_title;
        }
        default:
            return "";
    }
}

static uint8_t home_pump_mode_common_dot_index_get(uint8_t mode)
{
    if (!PUMP_IS_COMMON_MODE(mode)) {
        return 0;
    }
    return (uint8_t)(mode - BREAST_PUMP_NORMAL_MODE_LOWER);
}

static uint8_t home_pump_mode_bf_dot_index_get(uint8_t mode)
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

static uint8_t home_pump_mode_bf_dot_count_get(bool custom_active)
{
    return custom_active ? HOME_MODE_SELECT_DOT_MAX : 2;
}

static uint8_t home_pump_mode_select_dot_index_get(home_custom_ui_info_t *info, uint8_t mode)
{
    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
        return home_pump_mode_bf_dot_index_get(mode);
    }
    return home_pump_mode_common_dot_index_get(mode);
}

static lv_coord_t home_pump_mode_select_capsule_y_get(const home_custom_ui_info_t *info)
{
    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
        return HOME_MODE_BF_CAPSULE_Y;
    }
    return HOME_MODE_CAPSULE_Y;
}

static bool home_pump_mode_select_ui_active(home_pump_mode_display_type_e display_type, uint8_t mode)
{
    if (display_type == HOME_PUMP_MODE_DISPLAY_NORMAL_MODE && PUMP_IS_COMMON_MODE(mode)) {
        return true;
    }
    if (display_type == HOME_PUMP_MODE_DISPLAY_BF_MODE && home_pump_mode_is_bf_select_mode(mode)) {
        return true;
    }
    return false;
}

static uint8_t home_pump_mode_select_capsule_style_get(home_pump_mode_display_type_e display_type,
    uint8_t mode)
{
    if (display_type == HOME_PUMP_MODE_DISPLAY_BF_MODE && home_pump_mode_is_bf_select_mode(mode)) {
        return HOME_MODE_CAPSULE_STYLE_BF;
    }
    if (display_type == HOME_PUMP_MODE_DISPLAY_NORMAL_MODE && PUMP_IS_COMMON_MODE(mode)) {
        return HOME_MODE_CAPSULE_STYLE_COMMON;
    }
    return HOME_MODE_CAPSULE_STYLE_NONE;
}

static void home_pump_mode_select_layout_target_get(const home_custom_ui_info_t *info, bool shift_down,
    int16_t *label_offset, int16_t *time_offset)
{
    uint8_t mode = info->mode.new_index;

    if (!shift_down) {
        *label_offset = 0;
        *time_offset = 0;
        return;
    }

#if HOME_MODE_SELECT_CAPSULE_ENABLE
    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_COMMON) {
        *time_offset = HOME_MODE_SELECT_COMMON_TIME_Y_OFFSET;
        if (mode == BREAST_PUMP_STIMULATION_MODE) {
            *label_offset = HOME_MODE_SELECT_COMMON_STIM_LABEL_Y_OFFSET;
        } else if (mode == BREAST_PUMP_LACTATION_MODE) {
            *label_offset = HOME_MODE_SELECT_COMMON_EXPR_LABEL_Y_OFFSET;
        } else if (mode == BREAST_PUMP_MIX_MODE) {
            *label_offset = HOME_MODE_SELECT_COMMON_MIX_LABEL_Y_OFFSET;
        } else {
            *label_offset = 0;
        }
        return;
    }

    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
        if (mode == BREAST_PUMP_MILK_BOOST_MODE) {
            *label_offset = HOME_MODE_SELECT_BF_MB_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_BF_MB_TIME_Y_OFFSET;
        } else if (mode == BREAST_PUMP_PP_MILK_TRACK_MODE) {
            *label_offset = HOME_MODE_SELECT_BF_PP_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_BF_PP_TIME_Y_OFFSET;
        } else if (mode == BREAST_PUMP_CUSTOM_MODE) {
            *label_offset = HOME_MODE_SELECT_BF_CUSTOM_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_BF_CUSTOM_TIME_Y_OFFSET;
        } else {
            *label_offset = 0;
            *time_offset = 0;
        }
        return;
    }

    *label_offset = 0;
    *time_offset = 0;
#elif HOME_MODE_SELECT_CAROUSEL_ENABLE
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            *label_offset = HOME_MODE_SELECT_STIM_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_STIM_TIME_Y_OFFSET;
            break;
        case BREAST_PUMP_LACTATION_MODE:
            *label_offset = HOME_MODE_SELECT_EXPR_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_EXPR_TIME_Y_OFFSET;
            break;
        case BREAST_PUMP_MIX_MODE:
            *label_offset = HOME_MODE_SELECT_MIX_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_MIX_TIME_Y_OFFSET;
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            *label_offset = HOME_MODE_SELECT_MB_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_MB_TIME_Y_OFFSET;
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            *label_offset = HOME_MODE_SELECT_PP_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_PP_TIME_Y_OFFSET;
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            *label_offset = HOME_MODE_SELECT_CUSTOM_LABEL_Y_OFFSET;
            *time_offset = HOME_MODE_SELECT_CUSTOM_TIME_Y_OFFSET;
            break;
        default:
            *label_offset = 0;
            *time_offset = 0;
            break;
    }
#endif
}

static void home_pump_mode_anim_exec_opa(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
}

static void home_pump_mode_anim_exec_y(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_set_y(obj, (lv_coord_t)v);
}

static void home_pump_mode_obj_start_opa_anim(lv_obj_t *obj, lv_opa_t from, lv_opa_t to, bool show)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }

    if (show) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
    lv_anim_del(obj, home_pump_mode_anim_exec_opa);
    lv_obj_set_style_opa(obj, from, 0);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_time(&anim, HOME_MODE_SELECT_ANIM_TIME_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, home_pump_mode_anim_exec_opa);
    lv_anim_set_values(&anim, from, to);
    lv_anim_start(&anim);
}

static void home_pump_mode_select_label_offset_anim_exec(void *var, int32_t v)
{
    home_custom_ui_info_t *info = (home_custom_ui_info_t *)var;
    info->mode.select_label_y_offset = (int16_t)v;
    home_custom_pump_label_apply_select_offset(info);
}

static void home_pump_mode_select_time_offset_anim_exec(void *var, int32_t v)
{
    home_custom_ui_info_t *info = (home_custom_ui_info_t *)var;
    info->mode.select_time_y_offset = (int16_t)v;
    home_custom_pump_label_apply_select_offset(info);
}

static void home_pump_mode_obj_start_offset_anim(home_custom_ui_info_t *info,
    lv_anim_exec_xcb_t exec_cb, int32_t from, int32_t to)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, info);
    lv_anim_set_time(&anim, HOME_MODE_SELECT_ANIM_TIME_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, exec_cb);
    lv_anim_set_values(&anim, from, to);
    lv_anim_start(&anim);
}

static void home_pump_mode_select_layout_shift(home_custom_ui_info_t *info, bool shift_down, bool anim)
{
    int16_t target_label = 0;
    int16_t target_time = 0;

    home_pump_mode_select_layout_target_get(info, shift_down, &target_label, &target_time);

    if (!anim || info->force_update) {
        info->mode.select_label_y_offset = target_label;
        info->mode.select_time_y_offset = target_time;
        home_custom_pump_label_apply_select_offset(info);
        return;
    }

    lv_anim_del(info, home_pump_mode_select_label_offset_anim_exec);
    lv_anim_del(info, home_pump_mode_select_time_offset_anim_exec);
    home_pump_mode_obj_start_offset_anim(info, home_pump_mode_select_label_offset_anim_exec,
        info->mode.select_label_y_offset, target_label);
    if (info->mode.select_time_y_offset != target_time) {
        home_pump_mode_obj_start_offset_anim(info, home_pump_mode_select_time_offset_anim_exec,
            info->mode.select_time_y_offset, target_time);
    }
}

#if !HOME_MODE_SELECT_CAPSULE_ENABLE
static uint8_t home_pump_mode_neighbor_next_get(const home_custom_ui_info_t *info, uint8_t mode)
{
    const bool custom_active = info->mode.custom_mode_active != 0;

    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
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

static uint8_t home_pump_mode_neighbor_prev_get(const home_custom_ui_info_t *info, uint8_t mode)
{
    const bool custom_active = info->mode.custom_mode_active != 0;

    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
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

static int8_t home_pump_mode_select_slide_dir_get(const home_custom_ui_info_t *info)
{
    const uint8_t cur = info->mode.index;
    const uint8_t next = info->mode.new_index;

    if (next == home_pump_mode_neighbor_next_get(info, cur)) {
        return 1;
    }
    if (next == home_pump_mode_neighbor_prev_get(info, cur)) {
        return -1;
    }
    return 0;
}

static const char *home_pump_mode_select_name_get(home_custom_ui_info_t *info, uint8_t mode)
{
    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
        return home_pump_mode_bf_name_get(info, mode);
    }
    return home_pump_mode_common_name_get(mode);
}

static void home_pump_mode_select_title_style_apply(lv_obj_t *label, bool active)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return;
    }

    lv_obj_set_style_text_color(label, lv_color_hex(active ?
        HOME_MODE_TITLE_COLOR : HOME_MODE_SELECT_CAROUSEL_SIDE_COLOR), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    if (active) {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_26_bold, 0);
    } else {
        lv_obj_set_style_text_font(label, &lv_font_aeonik_soft_22_semibold, 0);
    }
}

static lv_coord_t home_pump_mode_select_title_half_width_get(lv_obj_t *label)
{
    if (label == NULL || !lv_obj_is_valid(label)) {
        return 0;
    }

    lv_obj_update_layout(label);
    const lv_coord_t width = lv_obj_get_width(label);
    return width > 0 ? (lv_coord_t)(width / 2) : 0;
}

static lv_coord_t home_pump_mode_select_carousel_step_get(home_custom_ui_info_t *info, int8_t dir)
{
    lv_obj_t *prev = info->mode.select_titles[HOME_MODE_SELECT_SLOT_PREV];
    lv_obj_t *cur = info->mode.select_titles[HOME_MODE_SELECT_SLOT_CUR];
    lv_obj_t *next = info->mode.select_titles[HOME_MODE_SELECT_SLOT_NEXT];

    if (dir == 1) {
        return home_pump_mode_select_title_half_width_get(prev) +
            HOME_MODE_SELECT_TITLE_GAP + home_pump_mode_select_title_half_width_get(cur);
    }
    if (dir == -1) {
        return home_pump_mode_select_title_half_width_get(cur) +
            HOME_MODE_SELECT_TITLE_GAP + home_pump_mode_select_title_half_width_get(next);
    }
    return 0;
}

static void home_pump_mode_select_carousel_realign(home_custom_ui_info_t *info)
{
    const lv_coord_t slide = info->mode.select_title_slide_offset;
    lv_obj_t *labels[3] = {
        info->mode.select_titles[HOME_MODE_SELECT_SLOT_PREV],
        info->mode.select_titles[HOME_MODE_SELECT_SLOT_CUR],
        info->mode.select_titles[HOME_MODE_SELECT_SLOT_NEXT],
    };
    lv_coord_t half_w[3];

    for (uint8_t i = 0; i < 3; i++) {
        half_w[i] = home_pump_mode_select_title_half_width_get(labels[i]);
    }

    const lv_coord_t center_x = lv_obj_get_width(info->root) / 2;
    const lv_coord_t x_mid = center_x - slide;
    lv_coord_t x_pos[3];

    x_pos[HOME_MODE_SELECT_SLOT_CUR] = x_mid;
    x_pos[HOME_MODE_SELECT_SLOT_PREV] = x_mid - half_w[HOME_MODE_SELECT_SLOT_CUR] -
        HOME_MODE_SELECT_TITLE_GAP - half_w[HOME_MODE_SELECT_SLOT_PREV];
    x_pos[HOME_MODE_SELECT_SLOT_NEXT] = x_mid + half_w[HOME_MODE_SELECT_SLOT_CUR] +
        HOME_MODE_SELECT_TITLE_GAP + half_w[HOME_MODE_SELECT_SLOT_NEXT];

    for (uint8_t i = 0; i < 3; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            continue;
        }
        const lv_coord_t title_y = (i == HOME_MODE_SELECT_SLOT_CUR) ?
            HOME_MODE_SELECT_TITLE_ACTIVE_Y : HOME_MODE_SELECT_TITLE_INACTIVE_Y;
        lv_obj_align(labels[i], LV_ALIGN_TOP_MID, x_pos[i] - center_x, title_y);
    }
}

static void home_pump_mode_select_carousel_create(home_custom_ui_info_t *info)
{
    for (uint8_t i = 0; i < 3; i++) {
        if (info->mode.select_titles[i] != NULL && lv_obj_is_valid(info->mode.select_titles[i])) {
            continue;
        }
        info->mode.select_titles[i] = lv_label_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.select_titles[i]);
        home_pump_mode_select_title_style_apply(info->mode.select_titles[i],
            i == HOME_MODE_SELECT_SLOT_CUR);
        lv_obj_add_flag(info->mode.select_titles[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void home_pump_mode_select_carousel_content_update_for_center(home_custom_ui_info_t *info,
    uint8_t center_mode)
{
    const uint8_t prev = home_pump_mode_neighbor_prev_get(info, center_mode);
    const uint8_t next = home_pump_mode_neighbor_next_get(info, center_mode);
    const uint8_t modes[3] = { prev, center_mode, next };

    home_pump_mode_select_carousel_create(info);
    for (uint8_t i = 0; i < 3; i++) {
        lv_obj_t *label = info->mode.select_titles[i];
        if (label == NULL) {
            continue;
        }
        lv_label_set_text(label, home_pump_mode_select_name_get(info, modes[i]));
        home_pump_mode_select_title_style_apply(label, i == HOME_MODE_SELECT_SLOT_CUR);
    }
    home_pump_mode_select_carousel_realign(info);
}

static void home_pump_mode_select_carousel_content_update(home_custom_ui_info_t *info)
{
    home_pump_mode_select_carousel_content_update_for_center(info, info->mode.new_index);
}

static void home_pump_mode_select_carousel_slide_ready_cb(lv_anim_t *anim)
{
    home_custom_ui_info_t *info = (home_custom_ui_info_t *)lv_anim_get_user_data(anim);
    if (info == NULL) {
        return;
    }

    info->mode.select_title_slide_offset = 0;
    info->mode.select_slide_dir = 0;
    home_pump_mode_select_carousel_content_update(info);
}

static void home_pump_mode_select_carousel_slide_exec(void *var, int32_t v);

static void home_pump_mode_select_carousel_slide_stop(home_custom_ui_info_t *info)
{
    lv_anim_del(info, home_pump_mode_select_carousel_slide_exec);
    info->mode.select_title_slide_offset = 0;
    info->mode.select_slide_dir = 0;
}

static void home_pump_mode_select_carousel_visible_set(home_custom_ui_info_t *info, bool visible)
{
    for (uint8_t i = 0; i < 3; i++) {
        home_pump_mode_obj_visible_set(info->mode.select_titles[i], visible);
    }
}

static void home_pump_mode_select_carousel_slide_exec(void *var, int32_t v)
{
    home_custom_ui_info_t *info = (home_custom_ui_info_t *)var;
    info->mode.select_title_slide_offset = (int16_t)v;
    home_pump_mode_select_carousel_realign(info);
}

static void home_pump_mode_select_carousel_slide(home_custom_ui_info_t *info, bool anim)
{
    const int8_t dir = info->mode.select_slide_dir;
    uint8_t old_center = info->mode.new_index;

    home_pump_mode_select_carousel_slide_stop(info);
    home_pump_mode_select_carousel_visible_set(info, true);

    if (dir == 1) {
        old_center = home_pump_mode_neighbor_prev_get(info, info->mode.new_index);
    } else if (dir == -1) {
        old_center = home_pump_mode_neighbor_next_get(info, info->mode.new_index);
    }

    if (!anim || info->force_update || dir == 0) {
        home_pump_mode_select_carousel_content_update(info);
        return;
    }

    home_pump_mode_select_carousel_content_update_for_center(info, old_center);
    const lv_coord_t step = home_pump_mode_select_carousel_step_get(info, dir);
    if (step == 0) {
        home_pump_mode_select_carousel_content_update(info);
        return;
    }
    const int32_t target = (int32_t)dir * step;

    lv_anim_t slide_anim;
    lv_anim_init(&slide_anim);
    lv_anim_set_var(&slide_anim, info);
    lv_anim_set_user_data(&slide_anim, info);
    lv_anim_set_time(&slide_anim, HOME_MODE_SELECT_ANIM_TIME_MS);
    lv_anim_set_path_cb(&slide_anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&slide_anim, home_pump_mode_select_carousel_slide_exec);
    lv_anim_set_values(&slide_anim, 0, target);
    lv_anim_set_ready_cb(&slide_anim, home_pump_mode_select_carousel_slide_ready_cb);
    lv_anim_start(&slide_anim);
}

static void home_pump_mode_select_carousel_enter(home_custom_ui_info_t *info, bool anim)
{
    (void)anim;
    home_pump_mode_select_carousel_slide_stop(info);
    home_pump_mode_select_carousel_content_update(info);
    home_pump_mode_select_carousel_visible_set(info, true);
}

static void home_pump_mode_select_carousel_exit(home_custom_ui_info_t *info)
{
    home_pump_mode_select_carousel_slide_stop(info);
    home_pump_mode_select_carousel_visible_set(info, false);
}

static void home_pump_mode_select_carousel_refresh(home_custom_ui_info_t *info, bool mode_index_changed,
    bool anim)
{
    if (mode_index_changed) {
        home_pump_mode_select_carousel_slide(info, anim);
        return;
    }
    home_pump_mode_select_carousel_content_update(info);
    home_pump_mode_select_carousel_visible_set(info, true);
}
#endif

#if HOME_MODE_SELECT_CAPSULE_ENABLE
static void home_pump_mode_capsule_create(home_custom_ui_info_t *info)
{
    bool capsule_newly_created = false;

    if (info->mode.title_capsule == NULL || !lv_obj_is_valid(info->mode.title_capsule)) {
        info->mode.title_capsule = lv_obj_create(info->root);
        EXCEPTION_CHECK_VOID(info->mode.title_capsule);
        lv_obj_remove_style_all(info->mode.title_capsule);
        lv_obj_set_style_radius(info->mode.title_capsule, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(info->mode.title_capsule, lv_color_hex(HOME_MODE_CAPSULE_BG_COLOR), 0);
        lv_obj_set_style_bg_opa(info->mode.title_capsule, LV_OPA_COVER, 0);
        lv_obj_clear_flag(info->mode.title_capsule, LV_OBJ_FLAG_SCROLLABLE);
        capsule_newly_created = true;
    }

    if (info->mode.capsule_title == NULL || !lv_obj_is_valid(info->mode.capsule_title)) {
        info->mode.capsule_title = lv_label_create(info->mode.title_capsule);
        EXCEPTION_CHECK_VOID(info->mode.capsule_title);
        lv_obj_set_style_text_color(info->mode.capsule_title, lv_color_hex(HOME_MODE_CAPSULE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->mode.capsule_title, &lv_font_aeonik_soft_20_bold, 0);
        lv_obj_set_style_text_align(info->mode.capsule_title, LV_TEXT_ALIGN_LEFT, 0);
    }

    if (info->mode.capsule_icon == NULL || !lv_obj_is_valid(info->mode.capsule_icon)) {
        info->mode.capsule_icon = lv_image_create(info->mode.title_capsule);
        EXCEPTION_CHECK_VOID(info->mode.capsule_icon);
        lv_obj_add_flag(info->mode.capsule_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (capsule_newly_created) {
        lv_obj_add_flag(info->mode.title_capsule, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(info->mode.title_capsule, LV_OPA_TRANSP, 0);
    }
}

static void home_pump_mode_dots_capsule_size_get(const home_custom_ui_info_t *info,
    lv_coord_t *capsule_width, lv_coord_t *capsule_height)
{
    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
        *capsule_width = HOME_MODE_BF_CAPSULE_W;
        *capsule_height = HOME_MODE_BF_CAPSULE_H;
        return;
    }
    *capsule_width = HOME_MODE_CAPSULE_W;
    *capsule_height = HOME_MODE_CAPSULE_H;
}

static void home_pump_mode_dots_create(home_custom_ui_info_t *info)
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
    home_pump_mode_dots_capsule_size_get(info, &capsule_width, &capsule_height);

    lv_coord_t total_width = dot_count * HOME_MODE_DOT_SIZE + (dot_count - 1) * HOME_MODE_DOT_GAP;
    lv_coord_t start_x = (capsule_width - total_width) / 2;
    lv_coord_t dot_y = capsule_height - HOME_MODE_CAPSULE_DOT_BOTTOM_PAD - HOME_MODE_DOT_SIZE;

    for (uint8_t i = 0; i < HOME_MODE_SELECT_DOT_MAX; i++) {
        if (info->mode.select_dots[i] != NULL && lv_obj_is_valid(info->mode.select_dots[i]) &&
            lv_obj_get_parent(info->mode.select_dots[i]) != capsule) {
            lv_obj_del(info->mode.select_dots[i]);
            info->mode.select_dots[i] = NULL;
        }
        if (info->mode.select_dots[i] == NULL || !lv_obj_is_valid(info->mode.select_dots[i])) {
            info->mode.select_dots[i] = lv_obj_create(capsule);
            EXCEPTION_CHECK_VOID(info->mode.select_dots[i]);
            lv_obj_remove_style_all(info->mode.select_dots[i]);
            lv_obj_set_size(info->mode.select_dots[i], HOME_MODE_DOT_SIZE, HOME_MODE_DOT_SIZE);
            lv_obj_set_style_radius(info->mode.select_dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(info->mode.select_dots[i], lv_color_hex(HOME_MODE_DOT_ACTIVE_COLOR), 0);
            lv_obj_set_style_bg_opa(info->mode.select_dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(info->mode.select_dots[i], 0, 0);
            lv_obj_clear_flag(info->mode.select_dots[i], LV_OBJ_FLAG_SCROLLABLE);
        }
        if (i >= dot_count) {
            home_pump_mode_obj_visible_set(info->mode.select_dots[i], false);
            continue;
        }
        lv_obj_clear_flag(info->mode.select_dots[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(info->mode.select_dots[i], LV_ALIGN_TOP_LEFT,
            start_x + i * (HOME_MODE_DOT_SIZE + HOME_MODE_DOT_GAP), dot_y);
    }
}

static void home_pump_mode_dots_update(home_custom_ui_info_t *info, uint8_t active_idx)
{
    home_pump_mode_dots_create(info);
    if (active_idx >= info->mode.select_dot_count) {
        active_idx = 0;
    }

    for (uint8_t i = 0; i < HOME_MODE_SELECT_DOT_MAX; i++) {
        lv_obj_t *dot = info->mode.select_dots[i];
        if (dot == NULL || !lv_obj_is_valid(dot)) {
            continue;
        }
        if (i >= info->mode.select_dot_count) {
            home_pump_mode_obj_visible_set(dot, false);
            continue;
        }
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(dot, lv_color_hex(i == active_idx ?
            HOME_MODE_DOT_ACTIVE_COLOR : HOME_MODE_DOT_INACTIVE_COLOR), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    }
}

static void home_pump_mode_capsule_common_layout(home_custom_ui_info_t *info, const char *title)
{
    home_pump_mode_capsule_create(info);
    if (info->mode.title_capsule == NULL || info->mode.capsule_title == NULL) {
        return;
    }

    home_pump_mode_obj_visible_set(info->mode.capsule_icon, false);
    lv_label_set_text(info->mode.capsule_title, title);
    lv_obj_set_style_text_font(info->mode.capsule_title, &lv_font_aeonik_soft_20_bold, 0);
    lv_obj_set_style_text_align(info->mode.capsule_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(info->mode.title_capsule, HOME_MODE_CAPSULE_W, HOME_MODE_CAPSULE_H);
    lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0, HOME_MODE_CAPSULE_Y);
    lv_obj_align(info->mode.capsule_title, LV_ALIGN_TOP_MID, 0, HOME_MODE_COMMON_CAPSULE_TITLE_Y);
    home_pump_mode_dots_create(info);
}

static void home_pump_mode_capsule_bf_layout(home_custom_ui_info_t *info, const char *title)
{
    home_pump_mode_capsule_create(info);
    if (info->mode.title_capsule == NULL || info->mode.capsule_title == NULL ||
        info->mode.capsule_icon == NULL) {
        return;
    }

    lv_label_set_text(info->mode.capsule_title, title);
    lv_obj_set_style_text_font(info->mode.capsule_title, &lv_font_aeonik_soft_20_bold, 0);
    lv_obj_set_style_text_align(info->mode.capsule_title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_size(info->mode.title_capsule, HOME_MODE_BF_CAPSULE_W, HOME_MODE_BF_CAPSULE_H);
    lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0, HOME_MODE_BF_CAPSULE_Y);

    lv_obj_clear_flag(info->mode.capsule_icon, LV_OBJ_FLAG_HIDDEN);
    ui_img_baj_theme_set_src(info->ui, info->mode.capsule_icon, CUSTOM_RES_ID_PUMP_RHYTHM_WHITE_ICON,
        RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    home_pump_mode_bf_capsule_icon_title_layout(info->mode.title_capsule, info->mode.capsule_icon,
        info->mode.capsule_title, HOME_MODE_BF_CAPSULE_W);
    home_pump_mode_dots_create(info);
}

static void home_pump_mode_capsule_layout(home_custom_ui_info_t *info, uint8_t mode)
{
    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
        home_pump_mode_capsule_bf_layout(info, home_pump_mode_bf_name_get(info, mode));
        return;
    }
    home_pump_mode_capsule_common_layout(info, home_pump_mode_common_name_get(mode));
}
#endif

static void home_pump_mode_default_title_restore(home_custom_ui_info_t *info)
{
    char custom_str[24] = {0};

    switch (info->mode.index) {
        case BREAST_PUMP_STIMULATION_MODE:
            home_pump_mode_title_update(info, "Stimulation", 0);
            break;
        case BREAST_PUMP_LACTATION_MODE:
            home_pump_mode_title_update(info, "Expression", 0);
            break;
        case BREAST_PUMP_MIX_MODE:
            home_pump_mode_title_update(info, "Mixed", 0);
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            if (strlen(info->mode.custom_id_str) > 0) {
                sprintf(custom_str, "Customized %s", info->mode.custom_id_str);
            } else {
                sprintf(custom_str, "Customized 01");
            }
            home_pump_mode_title_update(info, custom_str, CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            home_pump_mode_title_update(info, "Milk Boost", CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            home_pump_mode_title_update(info, "Power Pumping", CUSTOM_RES_ID_PUMP_RHYTHM_ICON);
            break;
        default:
            break;
    }
}

static void home_pump_mode_select_state_sync(home_custom_ui_info_t *info,
    home_pump_mode_display_type_e display_type)
{
    info->mode.select_capsule_style = home_pump_mode_select_capsule_style_get(display_type,
        info->mode.new_index);
    info->mode.custom_mode_active = home_pump_mode_custom_active_get(info) ? 1 : 0;
#if HOME_MODE_SELECT_CAPSULE_ENABLE
    if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_BF) {
        info->mode.select_dot_count = home_pump_mode_bf_dot_count_get(info->mode.custom_mode_active != 0);
    } else if (info->mode.select_capsule_style == HOME_MODE_CAPSULE_STYLE_COMMON) {
        info->mode.select_dot_count = HOME_MODE_SELECT_DOT_MAX;
    } else {
        info->mode.select_dot_count = 0;
    }
#endif
}

static void home_pump_mode_select_default_hide(home_custom_ui_info_t *info, bool anim)
{
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        if (!anim || info->force_update) {
            home_pump_mode_obj_visible_set(info->mode.title, false);
        } else {
            home_pump_mode_obj_start_opa_anim(info->mode.title, LV_OPA_COVER, LV_OPA_TRANSP, false);
        }
    }
    if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon)) {
        if (!anim || info->force_update) {
            home_pump_mode_obj_visible_set(info->mode.icon, false);
        } else {
            home_pump_mode_obj_start_opa_anim(info->mode.icon, LV_OPA_COVER, LV_OPA_TRANSP, false);
        }
    }
}

#if HOME_MODE_SELECT_CAROUSEL_ENABLE
static void home_pump_mode_select_carousel_default_cut(home_custom_ui_info_t *info)
{
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_anim_del(info->mode.title, NULL);
        lv_obj_set_style_opa(info->mode.title, LV_OPA_COVER, 0);
        home_pump_mode_obj_visible_set(info->mode.title, false);
    }
    if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon)) {
        lv_anim_del(info->mode.icon, NULL);
        lv_obj_set_style_opa(info->mode.icon, LV_OPA_COVER, 0);
        home_pump_mode_obj_visible_set(info->mode.icon, false);
    }
}
#endif

#if HOME_MODE_SELECT_CAPSULE_ENABLE
static void home_pump_mode_select_group_visible_set(home_custom_ui_info_t *info, bool visible, lv_opa_t opa)
{
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        home_pump_mode_obj_visible_set(info->mode.title_capsule, visible);
        lv_obj_set_style_opa(info->mode.title_capsule, opa, 0);
    }
}
#endif

static void home_pump_mode_select_label_time_refresh(home_custom_ui_info_t *info)
{
    info->label.force_update = true;
}

static void home_pump_mode_select_enter(home_custom_ui_info_t *info, bool anim)
{
#if HOME_MODE_SELECT_CAPSULE_ENABLE
    lv_coord_t capsule_y = home_pump_mode_select_capsule_y_get(info);
    uint8_t dot_idx = home_pump_mode_select_dot_index_get(info, info->mode.new_index);

    home_pump_mode_capsule_layout(info, info->mode.new_index);
    home_pump_mode_dots_update(info, dot_idx);
    home_pump_mode_select_layout_shift(info, true, anim);
    home_pump_mode_select_default_hide(info, anim);
    home_pump_mode_select_label_time_refresh(info);

    if (!anim || info->force_update) {
        home_pump_mode_select_group_visible_set(info, true, LV_OPA_COVER);
        if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
            lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0, capsule_y);
        }
        return;
    }

    home_pump_mode_select_group_visible_set(info, true, LV_OPA_TRANSP);
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0,
            capsule_y - HOME_MODE_SELECT_ENTER_Y_OFFSET);
        lv_anim_del(info->mode.title_capsule, home_pump_mode_anim_exec_y);
        lv_anim_del(info->mode.title_capsule, home_pump_mode_anim_exec_opa);

        lv_anim_t anim_y;
        lv_anim_init(&anim_y);
        lv_anim_set_var(&anim_y, info->mode.title_capsule);
        lv_anim_set_time(&anim_y, HOME_MODE_SELECT_ANIM_TIME_MS);
        lv_anim_set_path_cb(&anim_y, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&anim_y, home_pump_mode_anim_exec_y);
        lv_anim_set_values(&anim_y, capsule_y - HOME_MODE_SELECT_ENTER_Y_OFFSET, capsule_y);
        lv_anim_start(&anim_y);

        home_pump_mode_obj_start_opa_anim(info->mode.title_capsule, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
#else
    home_pump_mode_select_carousel_default_cut(info);
    home_pump_mode_select_carousel_enter(info, anim);
    home_pump_mode_select_layout_shift(info, true, anim);
    home_pump_mode_select_label_time_refresh(info);
#endif
}

static void home_pump_mode_select_exit(home_custom_ui_info_t *info, bool anim, uint8_t prev_capsule_style)
{
    (void)prev_capsule_style;
    home_pump_mode_select_layout_shift(info, false, anim);

#if HOME_MODE_SELECT_CAPSULE_ENABLE
    if (!anim || info->force_update) {
        home_pump_mode_select_group_visible_set(info, false, LV_OPA_TRANSP);
        home_pump_mode_default_title_restore(info);
        home_pump_mode_select_label_time_refresh(info);
        return;
    }

    home_pump_mode_default_title_restore(info);
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        home_pump_mode_obj_start_opa_anim(info->mode.title, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
    if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon) &&
        !lv_obj_has_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN)) {
        home_pump_mode_obj_start_opa_anim(info->mode.icon, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }

    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        home_pump_mode_obj_start_opa_anim(info->mode.title_capsule, LV_OPA_COVER, LV_OPA_TRANSP, false);
    }
    home_pump_mode_select_label_time_refresh(info);
#else
    home_pump_mode_select_carousel_exit(info);

    if (!anim || info->force_update) {
        home_pump_mode_default_title_restore(info);
        home_pump_mode_select_label_time_refresh(info);
        return;
    }

    home_pump_mode_default_title_restore(info);
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_obj_clear_flag(info->mode.title, LV_OBJ_FLAG_HIDDEN);
        home_pump_mode_obj_start_opa_anim(info->mode.title, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
    if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon) &&
        !lv_obj_has_flag(info->mode.icon, LV_OBJ_FLAG_HIDDEN)) {
        home_pump_mode_obj_start_opa_anim(info->mode.icon, LV_OPA_TRANSP, LV_OPA_COVER, true);
    }
    home_pump_mode_select_label_time_refresh(info);
#endif
}

static void home_pump_mode_select_refresh(home_custom_ui_info_t *info, bool mode_index_changed, bool anim)
{
#if HOME_MODE_SELECT_CAPSULE_ENABLE
    uint8_t dot_idx = home_pump_mode_select_dot_index_get(info, info->mode.new_index);

    home_pump_mode_capsule_layout(info, info->mode.new_index);
    home_pump_mode_dots_update(info, dot_idx);
    home_pump_mode_obj_visible_set(info->mode.title, false);
    home_pump_mode_obj_visible_set(info->mode.icon, false);
    home_pump_mode_select_group_visible_set(info, true, LV_OPA_COVER);
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        lv_obj_align(info->mode.title_capsule, LV_ALIGN_TOP_MID, 0,
            home_pump_mode_select_capsule_y_get(info));
    }
#else
    home_pump_mode_obj_visible_set(info->mode.title, false);
    home_pump_mode_obj_visible_set(info->mode.icon, false);
    home_pump_mode_select_carousel_refresh(info, mode_index_changed, anim);
#endif
}

static void home_pump_mode_select_ui_apply(home_custom_ui_info_t *info,
    home_pump_mode_display_type_e display_type, bool mode_index_changed)
{
    const bool select_ui = home_pump_mode_select_ui_active(display_type, info->mode.new_index);
    const bool prev_select_ui = info->mode.select_ui_active != 0;
    const bool anim = !info->force_update;
    const uint8_t prev_style = info->mode.select_capsule_style;
#if HOME_MODE_SELECT_CAPSULE_ENABLE
    const uint8_t prev_dot_count = info->mode.select_dot_count;
#endif
    const bool prev_custom_active = info->mode.custom_mode_active;

    home_pump_mode_select_state_sync(info, display_type);

    if (select_ui && !prev_select_ui) {
        home_pump_mode_select_enter(info, anim);
    } else if (!select_ui && prev_select_ui) {
        home_pump_mode_select_exit(info, anim, prev_style);
    } else if (select_ui) {
        const bool style_changed = prev_style != info->mode.select_capsule_style;
#if HOME_MODE_SELECT_CAPSULE_ENABLE
        const bool dot_count_changed = prev_dot_count != info->mode.select_dot_count;
#else
        const bool dot_count_changed = false;
#endif
        const bool custom_changed = prev_custom_active != info->mode.custom_mode_active;

        if (style_changed || dot_count_changed || custom_changed || mode_index_changed) {
            home_pump_mode_select_refresh(info, mode_index_changed, anim);
            if (style_changed || mode_index_changed) {
                home_pump_mode_select_layout_shift(info, true, anim);
                home_pump_mode_select_label_time_refresh(info);
            } else if (dot_count_changed || custom_changed) {
                home_pump_mode_select_label_time_refresh(info);
            }
        }
    } else if (!select_ui && (prev_select_ui || info->force_update)) {
        if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
            home_pump_mode_obj_visible_set(info->mode.title, true);
            lv_obj_set_style_opa(info->mode.title, LV_OPA_COVER, 0);
        }
#if HOME_MODE_SELECT_CAPSULE_ENABLE
        home_pump_mode_select_group_visible_set(info, false, LV_OPA_TRANSP);
#else
        home_pump_mode_select_carousel_exit(info);
#endif
    }

    info->mode.select_ui_active = select_ui ? 1 : 0;
    info->mode.display_type = (uint8_t)display_type;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_pump_mode_ui_remove(home_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
#if HOME_MODE_SELECT_TRANSITION_ENABLE
    lv_anim_del(info, NULL);
    if (info->mode.title != NULL && lv_obj_is_valid(info->mode.title)) {
        lv_anim_del(info->mode.title, NULL);
    }
    if (info->mode.icon != NULL && lv_obj_is_valid(info->mode.icon)) {
        lv_anim_del(info->mode.icon, NULL);
    }
#if HOME_MODE_SELECT_CAPSULE_ENABLE
    if (info->mode.title_capsule != NULL && lv_obj_is_valid(info->mode.title_capsule)) {
        lv_anim_del(info->mode.title_capsule, NULL);
    }
#else
    info->mode.select_title_slide_offset = 0;
    info->mode.select_slide_dir = 0;
#endif
#endif
    ui_img_baj_release_decoded_buf(info->mode.icon);
#if HOME_MODE_SELECT_CAPSULE_ENABLE
    ui_img_baj_release_decoded_buf(info->mode.capsule_icon);
#endif
}

void home_custom_pump_mode_ui_update(home_custom_ui_info_t *info, bool blink_visible,
    home_pump_mode_display_type_e display_type)
{
    EXCEPTION_CHECK_VOID(info);

    #if IS_SUPPORT_HOME_CUSTOM_MODE_CMP
    info->mode.new_index = HOME_MODE_CMP_INDEX;
    #endif

    const bool mode_index_changed = info->force_update ||
        info->mode.index != info->mode.new_index;
#if HOME_MODE_SELECT_TRANSITION_ENABLE
    const bool select_ui = home_pump_mode_select_ui_active(display_type, info->mode.new_index);
    const bool select_ui_transition = (info->mode.select_ui_active != 0) != select_ui;
    const bool display_changed = info->mode.display_type != (uint8_t)display_type;
#endif

#if HOME_MODE_SELECT_TRANSITION_ENABLE
    if (!info->force_update && !mode_index_changed && !select_ui_transition && !display_changed) {
        (void)blink_visible;
        return;
    }
#else
    if (!info->force_update && !mode_index_changed) {
#if HOME_MODE_TITLE_BLINK_ENABLE
        home_pump_mode_title_blink_apply(info, blink_visible);
        home_pump_mode_icon_blink_apply(info, blink_visible);
#else
        (void)blink_visible;
#endif
        return;
    }
#endif

    if (mode_index_changed) {
#if HOME_MODE_SELECT_TRANSITION_ENABLE && !HOME_MODE_SELECT_CAPSULE_ENABLE
        if (info->mode.select_ui_active) {
            info->mode.select_slide_dir = home_pump_mode_select_slide_dir_get(info);
        } else {
            info->mode.select_slide_dir = 0;
        }
#endif
#if HOME_MODE_SELECT_CAPSULE_ENABLE
        home_pump_mode_capsule_create(info);
#endif
        home_pump_mode_apply_index_change(info);
    }

#if HOME_MODE_SELECT_TRANSITION_ENABLE
    home_pump_mode_select_ui_apply(info, display_type, mode_index_changed);
#else
    (void)display_type;
#if HOME_MODE_TITLE_BLINK_ENABLE
    home_pump_mode_title_blink_apply(info, blink_visible);
    home_pump_mode_icon_blink_apply(info, blink_visible);
#endif
#endif
}
