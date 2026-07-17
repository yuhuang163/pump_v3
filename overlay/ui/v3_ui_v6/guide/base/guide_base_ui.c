/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_BASE_GUIDE_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t guide_bg_data[] = {
    // #include "comparison_png2txt/guide_base_p11_360x360.txt"
    // #include "comparison_png2txt/guide_base_p40_360x360.txt"
    // #include "comparison_png2txt/guide_base_p41_360x360.txt"
    // #include "comparison_png2txt/guide_base_p42_360x360.txt"
    // #include "comparison_png2txt/guide_base_p43_360x360.txt"
    #include "comparison_png2txt/guide_base_p44_360x360.txt"
    // #include "comparison_png2txt/guide_base_p45_360x360.txt"
    // #include "comparison_png2txt/guide_base_p46_360x360.txt"
    // #include "comparison_png2txt/guide_base_p47_360x360.txt"
    // #include "comparison_png2txt/guide_base_p48_360x360.txt"
    // #include "comparison_png2txt/guide_base_p49_360x360.txt"
    // #include "comparison_png2txt/guide_base_p50_360x360.txt"
    // #include "comparison_png2txt/guide_base_p51_360x360.txt"
    // #include "comparison_png2txt/guide_base_p52_360x360.txt"
    // #include "comparison_png2txt/guide_base_p53_360x360.txt"
    // #include "comparison_png2txt/guide_base_p54_360x360.txt"
};
#endif

static guide_base_ui_info_t *guide_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_base_blink_state_update(guide_base_ui_info_t *info, uint8_t page_index)
{
    uint8_t new_left_level_blink = 0;
    uint8_t new_right_level_blink = 0;
    guide_base_level_display_type_e display_type = GUIDE_BASE_LEVEL_DISPLAY_DEFAULT;

    EXCEPTION_CHECK_VOID(info);

    switch (page_index) {
        case GUIDE_PAGE_LEVEL_BOTH:
            display_type = GUIDE_BASE_LEVEL_DISPLAY_BOTH_ADJUST;
            new_left_level_blink = 1;
            new_right_level_blink = 1;
            break;
        case GUIDE_PAGE_LEVEL_LEFT:
            display_type = GUIDE_BASE_LEVEL_DISPLAY_LEFT_ADJUST;
            new_left_level_blink = 1;
            break;
        case GUIDE_PAGE_LEVEL_RIGHT:
            display_type = GUIDE_BASE_LEVEL_DISPLAY_RIGHT_ADJUST;
            new_right_level_blink = 1;
            break;
        default:
            display_type = GUIDE_BASE_LEVEL_DISPLAY_DEFAULT;
            break;
    }

    info->level.left_blink_state = new_left_level_blink;
    info->level.right_blink_state = new_right_level_blink;
    info->level_display_type = display_type;
}

static void guide_base_root_init(void)
{
    if (guide_info->root == NULL || !lv_obj_is_valid(guide_info->root)) {
        guide_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(guide_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(guide_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(guide_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(guide_info->root, 0, 0);
    lv_obj_set_style_pad_all(guide_info->root, 0, 0);
    lv_obj_align(guide_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(guide_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

bool guide_base_level_theme_page_is(uint8_t page_index)
{
    return (page_index >= GUIDE_PAGE_LEVEL_BOTH_GUIDE &&
        page_index <= GUIDE_PAGE_LEVEL_RIGHT);
}

bool guide_base_level_theme_is_active(const guide_base_ui_info_t *info)
{
    return info != NULL && info->level_theme.layer != NULL &&
        lv_obj_is_valid(info->level_theme.layer);
}

#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
static void guide_base_level_theme_bg_create(guide_base_ui_info_t *info)
{
    if (info->level_theme.bg != NULL && lv_obj_is_valid(info->level_theme.bg)) {
        return;
    }

    info->level_theme.bg = lv_image_create(info->level_theme.layer);
    EXCEPTION_CHECK_VOID(info->level_theme.bg);
#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_COMMON_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
    lv_image_set_src(info->level_theme.bg, background_snapshot);
#else
    ui_img_png_theme_set_src(info->ui, info->level_theme.bg, BASE_RES_ID_COMMON_BG,
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
#endif
    lv_obj_align(info->level_theme.bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_move_background(info->level_theme.bg);
}
#endif

void guide_base_level_theme_ensure(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);

    if (guide_base_level_theme_is_active(info)) {
        return;
    }

    info->level_theme.layer = lv_obj_create(info->root);
    EXCEPTION_CHECK_VOID(info->level_theme.layer);
    lv_obj_set_size(info->level_theme.layer, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(info->level_theme.layer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(info->level_theme.layer, 0, 0);
    lv_obj_set_style_pad_all(info->level_theme.layer, 0, 0);
    lv_obj_clear_flag(info->level_theme.layer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(info->level_theme.layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(info->level_theme.layer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_move_background(info->level_theme.layer);

#if (IS_SUPPORT_BASE_GUIDE_COMP_BG == 0)
    guide_base_level_theme_bg_create(info);
#endif
}

void guide_base_level_theme_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);

    if (!guide_base_level_theme_is_active(info)) {
        return;
    }

    guide_pump_level_ui_remove(info);
#if !LV_USE_SNAPSHOT
    ui_img_png_release_decoded_buf(info->level_theme.bg);
#endif
    LVGL_OBJ_DELETE(info->level_theme.layer);
    memset(&info->level_theme, 0, sizeof(info->level_theme));
    memset(&info->level, 0, sizeof(info->level));
}

lv_obj_t *guide_base_level_theme_parent_get(const guide_base_ui_info_t *info)
{
    if (guide_base_level_theme_is_active(info)) {
        return info->level_theme.layer;
    }
    return guide_page_parent_get(info);
}

#if IS_SUPPORT_BASE_GUIDE_COMP_BG
static void guide_root_comp_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    if (guide_info->comp_bg == NULL || !lv_obj_is_valid(guide_info->comp_bg)) {
        guide_info->comp_bg = lv_image_create(guide_info->root);
        EXCEPTION_CHECK_VOID(guide_info->comp_bg);
        lv_obj_move_background(guide_info->comp_bg);
    }

    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(guide_info->comp_bg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(guide_info->comp_bg, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, guide_bg_data,
            sizeof(guide_bg_data), "png");
        lv_image_set_src(guide_info->comp_bg, (const void *)&bg_img_path);
        lv_obj_align(guide_info->comp_bg, LV_ALIGN_CENTER, 0, 0);
        lv_obj_move_background(guide_info->comp_bg);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
    (void)thiz;
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
guide_base_ui_info_t *guide_base_info_get(void)
{
    return guide_info;
}

void guide_base_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (guide_info == NULL) {
        guide_info = (guide_base_ui_info_t *)u_malloz(sizeof(guide_base_ui_info_t));
        EXCEPTION_CHECK_VOID(guide_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    guide_base_root_init();
    guide_info->ui = thiz;
    guide_info->force_update = true;
    guide_info->popup_ui_index = ID_POPUP_UI_NONE;
    guide_info->shown_page_index = GUIDE_PAGE_INDEX_INVALID;
    guide_info->page_index = GUIDE_PAGE_MAX;
    guide_info->label.text_id = GUIDE_PAGE_MAX;
#if IS_SUPPORT_BASE_GUIDE_WELCOME_ANIM
    guide_info->animation.shown_frame_res_id = BASE_RES_ID_GUIDE_ANIM_START - 1;
#endif
}

void guide_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && guide_info && guide_info->root);
    guide_page_ui_remove(guide_info);
    ui_popup_remove(thiz);
    LVGL_OBJ_DELETE(guide_info->root);
    u_free(guide_info);
    guide_info = NULL;
}

int guide_base_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && guide_info && guide_info->root);
    ui_proc_guide_t *proc = (ui_proc_guide_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);

#if IS_SUPPORT_BASE_GUIDE_COMP_BG
    guide_root_comp_bg_update(thiz);
    proc->page_index = GUIDE_PAGE_MODE_SELECT_GUIDE;
#endif
    guide_info->level.left.new_value = proc->setting.level[0];
    guide_info->level.right.new_value = proc->setting.level[1];
    guide_page_ui_update(guide_info, proc, thiz->priv.play_info.played_count);
    ui_popup_update(thiz, proc->popup_ui_type,
        guide_info->root, &guide_info->popup_ui_index);
    guide_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> guide fps: %d, page: %u", thiz->priv.fps_count, proc->page_index);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
