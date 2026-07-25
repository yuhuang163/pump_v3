/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep
#include "guide_custom_ui_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_page_content_reset(guide_custom_ui_info_t *info)
{
    guide_custom_level_guide_ui_remove(info);
    guide_custom_mode_select_ui_remove(info);
    guide_custom_rhythm_select_ui_remove(info);
    guide_custom_pump_label_ui_remove(info);
    guide_custom_ip_ui_on_page_reset(info);
    if (info->page != NULL && lv_obj_is_valid(info->page)) {
        LVGL_OBJ_DELETE(info->page);
    }
    info->page = NULL;
    memset(&info->level_guide, 0, sizeof(info->level_guide));
    memset(&info->mode_select, 0, sizeof(info->mode_select));
    memset(&info->rhythm_select, 0, sizeof(info->rhythm_select));
    memset(&info->label, 0, sizeof(info->label));
    memset(&info->page_num, 0, sizeof(info->page_num));
    info->label.text_id = GUIDE_PAGE_MAX;
}

static void guide_page_container_reset(guide_custom_ui_info_t *info)
{
    guide_custom_pump_animation_ui_remove(info);
    guide_custom_intro_ui_remove(info);
    guide_custom_key_guide_ui_remove(info);
    guide_custom_knob_guide_ui_remove(info);
    guide_custom_function_home_ui_remove(info);
    guide_custom_finish_ui_remove(info);
    guide_custom_level_theme_remove(info);
    guide_page_content_reset(info);
    memset(&info->animation, 0, sizeof(info->animation));
    memset(&info->intro, 0, sizeof(info->intro));
    memset(&info->key_guide, 0, sizeof(info->key_guide));
    memset(&info->knob_guide, 0, sizeof(info->knob_guide));
    memset(&info->function_home, 0, sizeof(info->function_home));
    memset(&info->finish, 0, sizeof(info->finish));
    info->animation.shown_frame_res_id = CUSTOM_RES_ID_GUIDE_IP_START - 1;
}

static void guide_page_container_create(guide_custom_ui_info_t *info)
{
    info->page = lv_obj_create(info->root);
    EXCEPTION_CHECK_VOID(info->page);
    lv_obj_set_size(info->page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(info->page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(info->page, 0, 0);
    lv_obj_set_style_pad_all(info->page, 0, 0);
    lv_obj_clear_flag(info->page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(info->page, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(info->page, LV_ALIGN_CENTER, 0, 0);
}

static void guide_page_common_bg_create(guide_custom_ui_info_t *info)
{
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    lv_obj_t *bg = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(bg);
    lv_image_set_src(bg, background_snapshot);
    lv_obj_align(bg, LV_ALIGN_CENTER, 0, 0);
#else
    (void)info;
#endif
}

static void guide_page_icon_create(guide_custom_ui_info_t *info, uint16_t res_id, int16_t x, int16_t y)
{
    lv_obj_t *icon = lv_image_create(guide_custom_page_parent_get(info));
    EXCEPTION_CHECK_VOID(icon);
    ui_img_baj_theme_set_src(info->ui, icon, res_id, RES_THEME_TYPE_CUSTOM, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_align(icon, LV_ALIGN_TOP_LEFT, x, y);
}

static void guide_page_level_theme_finish_build(guide_custom_ui_info_t *info, uint8_t page_index,
    bool reuse_level_ui)
{
    guide_custom_blink_state_update(info, page_index);
    if (!reuse_level_ui) {
        guide_custom_pump_level_ui_build(info);
    }
}

static void guide_page_build(guide_custom_ui_info_t *info, uint8_t page_index, bool reuse_level_ui)
{
    guide_page_container_create(info);

    switch (page_index) {
        case GUIDE_PAGE_WELCOME_ANIM:
            guide_custom_pump_animation_ui_build(info);
            break;
        case GUIDE_PAGE_INTRO:
            guide_custom_intro_ui_build(info);
            break;
        case GUIDE_PAGE_KEY_GUIDE:
            guide_custom_key_guide_ui_build(info);
            break;
        case GUIDE_PAGE_KNOB_GUIDE:
            guide_custom_knob_guide_ui_build(info);
            break;
        case GUIDE_PAGE_FUNCTION_HOME:
            guide_custom_function_home_ui_build(info);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
        case GUIDE_PAGE_LEVEL_LEFT_GUIDE:
        case GUIDE_PAGE_LEVEL_LEFT:
        case GUIDE_PAGE_LEVEL_RIGHT_GUIDE:
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_custom_level_guide_ui_build(info, page_index);
            guide_page_level_theme_finish_build(info, page_index, reuse_level_ui);
            break;
        case GUIDE_PAGE_MODE_SELECT_GUIDE:
            guide_custom_mode_select_guide_ui_build(info);
            break;
        case GUIDE_PAGE_MODE_SELECT:
            guide_custom_mode_select_ui_build(info);
            break;
        case GUIDE_PAGE_RHYTHM_SELECT_GUIDE:
            guide_custom_rhythm_select_guide_ui_build(info);
            break;
        case GUIDE_PAGE_RHYTHM_SELECT:
            guide_custom_rhythm_select_ui_build(info);
            break;
        case GUIDE_PAGE_PUMP_STOP_GUIDE:
            guide_custom_pump_stop_guide_ui_build(info);
            break;
        case GUIDE_PAGE_FINISH:
            guide_custom_finish_ui_build(info);
            break;
        default:
            guide_page_common_bg_create(info);
            guide_custom_pump_label_ui_build(info, page_index);
            break;
    }

    guide_custom_page_num_ui_build(info, page_index);
}

static void guide_page_switch(guide_custom_ui_info_t *info, uint8_t page_index)
{
    const bool new_is_theme = guide_custom_level_theme_page_is(page_index);
    const bool old_is_theme = guide_custom_level_theme_page_is(info->shown_page_index);
    const bool reuse_level_ui = new_is_theme && old_is_theme;
    const bool same_level_shell = reuse_level_ui &&
        guide_custom_level_guide_same_shell(info->shown_page_index, page_index);

    if (same_level_shell) {
        guide_custom_level_guide_ui_update(info, page_index);
    } else if (reuse_level_ui) {
        guide_page_content_reset(info);
    } else {
        guide_page_container_reset(info);
    }

    info->page_index = page_index;
    info->shown_page_index = page_index;
    if (!same_level_shell) {
        info->force_update = true;
    }

    if (new_is_theme) {
        guide_custom_level_theme_ensure(info);
    }

    if (!same_level_shell) {
        guide_page_build(info, page_index, reuse_level_ui);
    } else {
        guide_custom_blink_state_update(info, page_index);
    }
    guide_custom_ip_ui_update(info, page_index);
#if IS_SUPPORT_CUSTOM_GUIDE_COMP_BG
    if (info->comp_bg != NULL && lv_obj_is_valid(info->comp_bg)) {
        lv_obj_move_background(info->comp_bg);
    }
#endif
}

static void guide_page_dynamic_update(guide_custom_ui_info_t *info, ui_proc_guide_t *proc,
    uint16_t frame_index)
{
    switch (proc->page_index) {
        case GUIDE_PAGE_WELCOME_ANIM:
            guide_custom_pump_animation_ui_update(info, frame_index);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
        case GUIDE_PAGE_LEVEL_BOTH_INCREASE:
        case GUIDE_PAGE_LEVEL_BOTH_DECREASE:
        case GUIDE_PAGE_LEVEL_LEFT_GUIDE:
        case GUIDE_PAGE_LEVEL_LEFT:
        case GUIDE_PAGE_LEVEL_RIGHT_GUIDE:
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_custom_blink_state_update(info, proc->page_index);
            guide_custom_pump_level_ui_update(info, proc->state_blink_visible != 0);
            guide_custom_level_idle_countdown_ui_update(info, proc);
            break;
        case GUIDE_PAGE_MODE_SELECT:
            guide_custom_mode_select_ui_update(info, proc);
            break;
        case GUIDE_PAGE_RHYTHM_SELECT:
            guide_custom_rhythm_select_ui_update(info, proc);
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_custom_page_ui_remove(guide_custom_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_page_container_reset(info);
}

void guide_custom_page_ui_update(guide_custom_ui_info_t *info, ui_proc_guide_t *proc, uint16_t frame_index)
{
    EXCEPTION_CHECK_VOID(info && proc);

    if (info->shown_page_index != proc->page_index) {
        guide_page_switch(info, proc->page_index);
    }

    guide_page_dynamic_update(info, proc, frame_index);
    guide_custom_ip_ui_update(info, proc->page_index);
    info->force_update = false;
}
