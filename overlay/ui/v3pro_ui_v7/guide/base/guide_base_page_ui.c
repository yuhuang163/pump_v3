/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_page_content_reset(guide_base_ui_info_t *info)
{
    guide_qrcode_ui_remove(info);
    guide_intro_ui_remove(info);
    guide_pump_start_guide_ui_remove(info);
    guide_pump_start_ui_remove(info);
    guide_pump_pause_guide_ui_remove(info);
    guide_pump_pause_ui_remove(info);
    guide_pump_finish_guide_ui_remove(info);
    guide_pump_finish_hold_ui_remove(info);
    guide_pump_home_ui_remove(info);
    guide_mode_home_ui_remove(info);
    guide_mode_switch_guide_ui_remove(info);
    guide_mode_switch_ui_remove(info);
    guide_level_side_guide_ui_remove(info);
    guide_level_side_ui_remove(info);
    guide_level_side_done_ui_remove(info);
    guide_level_both_guide_ui_remove(info);
    guide_level_both_ui_remove(info);
    guide_level_both_done_ui_remove(info);
    guide_speed_guide_ui_remove(info);
    guide_speed_ui_remove(info);
    guide_speed_done_ui_remove(info);
    guide_finish_ui_remove(info);
    guide_page_num_ui_remove(&info->page_num);
    if (info->page != NULL && lv_obj_is_valid(info->page)) {
        LVGL_OBJ_DELETE(info->page);
    }
    info->page = NULL;
}

static void guide_page_container_create(guide_base_ui_info_t *info)
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

static void guide_page_build(guide_base_ui_info_t *info, uint8_t page_index)
{
    if (page_index == GUIDE_PAGE_PUMP_FINISH_HOLD) {
        guide_pump_finish_hold_ui_build(info);
        return;
    }
    if (page_index == GUIDE_PAGE_MODE_SWITCH) {
        guide_mode_switch_ui_build(info);
        return;
    }

    guide_page_container_create(info);

    switch (page_index) {
        case GUIDE_PAGE_QRCODE:
            guide_qrcode_ui_build(info);
            break;
        case GUIDE_PAGE_INTRO:
            guide_intro_ui_build(info);
            break;
        case GUIDE_PAGE_PUMP_START_GUIDE:
            guide_pump_start_guide_ui_build(info);
            break;
        case GUIDE_PAGE_PUMP_HOME_GUIDE:
            guide_pump_start_ui_build(info);
            break;
        case GUIDE_PAGE_PUMP_PAUSE_GUIDE:
            guide_pump_pause_guide_ui_build(info);
            break;
        case GUIDE_PAGE_PUMP_PAUSE:
            guide_pump_pause_ui_build(info);
            break;
        case GUIDE_PAGE_PUMP_FINISH_GUIDE:
            guide_pump_finish_guide_ui_build(info);
            break;
        case GUIDE_PAGE_PUMP_HOME:
            guide_pump_home_ui_build(info);
            break;
        case GUIDE_PAGE_MODE_SWITCH_GUIDE:
            guide_mode_switch_guide_ui_build(info);
            break;
        case GUIDE_PAGE_MODE_HOME:
            guide_mode_home_ui_build(info);
            break;
        case GUIDE_PAGE_LEVEL_SIDE_GUIDE:
            guide_level_side_guide_ui_build(info);
            break;
        case GUIDE_PAGE_LEVEL_SIDE:
            guide_level_side_ui_build(info);
            break;
        case GUIDE_PAGE_LEVEL_SIDE_DONE:
            guide_level_side_done_ui_build(info);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
            guide_level_both_guide_ui_build(info);
            break;
        case GUIDE_PAGE_LEVEL_BOTH:
            guide_level_both_ui_build(info);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_DONE:
            guide_level_both_done_ui_build(info);
            break;
        case GUIDE_PAGE_SPEED_GUIDE:
            guide_speed_guide_ui_build(info);
            break;
        case GUIDE_PAGE_SPEED:
            guide_speed_ui_build(info);
            break;
        case GUIDE_PAGE_SPEED_DONE:
            guide_speed_done_ui_build(info);
            break;
        case GUIDE_PAGE_FINISH:
            guide_finish_ui_build(info);
            break;
        default:
            break;
    }
}

static void guide_page_switch(guide_base_ui_info_t *info, uint8_t page_index)
{
    guide_page_content_reset(info);

    info->page_index = page_index;
    info->shown_page_index = page_index;
    info->force_update = true;

    guide_page_build(info, page_index);
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
    if (info->comp_bg != NULL && lv_obj_is_valid(info->comp_bg)) {
        lv_obj_move_background(info->comp_bg);
    }
#endif
}

static void guide_page_dynamic_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc,
    uint16_t frame_index)
{
    (void)frame_index;
    switch (proc->page_index) {
        case GUIDE_PAGE_QRCODE:
            guide_qrcode_ui_update(info);
            break;
        case GUIDE_PAGE_PUMP_HOME_GUIDE:
            guide_pump_start_ui_update(info, proc);
            break;
        case GUIDE_PAGE_PUMP_HOME:
            guide_pump_home_ui_update(info, proc);
            break;
        case GUIDE_PAGE_PUMP_FINISH_HOLD:
            guide_pump_finish_hold_ui_update(info);
            break;
        case GUIDE_PAGE_MODE_SWITCH_GUIDE:
            guide_mode_switch_guide_ui_update(info);
            break;
        case GUIDE_PAGE_MODE_SWITCH:
            guide_mode_switch_ui_update(info, proc);
            break;
        case GUIDE_PAGE_MODE_HOME:
            guide_mode_home_ui_update(info, proc);
            break;
        case GUIDE_PAGE_LEVEL_SIDE_GUIDE:
            guide_level_side_guide_ui_update(info);
            break;
        case GUIDE_PAGE_LEVEL_SIDE:
            guide_level_side_ui_update(info, proc);
            break;
        case GUIDE_PAGE_LEVEL_SIDE_DONE:
            guide_level_side_done_ui_update(info, proc);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
            guide_level_both_guide_ui_update(info);
            break;
        case GUIDE_PAGE_LEVEL_BOTH:
            guide_level_both_ui_update(info, proc);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_DONE:
            guide_level_both_done_ui_update(info, proc);
            break;
        case GUIDE_PAGE_SPEED_GUIDE:
            guide_speed_guide_ui_update(info);
            break;
        case GUIDE_PAGE_SPEED:
            guide_speed_ui_update(info, proc);
            break;
        case GUIDE_PAGE_SPEED_DONE:
            guide_speed_done_ui_update(info, proc);
            break;
        case GUIDE_PAGE_FINISH:
            guide_finish_ui_update(info);
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_page_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_page_content_reset(info);
}

void guide_page_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc, uint16_t frame_index)
{
    EXCEPTION_CHECK_VOID(info && proc);

    if (info->shown_page_index != proc->page_index) {
        guide_page_switch(info, proc->page_index);
    }

    guide_page_dynamic_update(info, proc, frame_index);
    info->force_update = false;
}
