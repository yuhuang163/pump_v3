/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"
#include "window_screen_pump.h"
#include "breast_pump_ui.h"

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_LEVEL_COMP_BG                         (0)

#define PUMP_TITLE_TEXT_COLOR                                 (0x533129)
#define PUMP_LEVEL_TEXT_COLOR                                 (0x533129)
#define PUMP_LEVEL_DIR_TEXT_COLOR                             (0xB35C65)
#define PUMP_LEVEL_MAX                                        (15)
#define PUMP_LEVEL_TITLE_TEXT_Y                               (92)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * title_text; /// 档位文字
    lv_obj_t * level_direction;    /// 等级方向
    lv_obj_t * level_text; /// 等级偏移文字
    lv_obj_t * level_range_text; /// 等级范围文字
    lv_obj_t * level_progress; /// 等级进度
    res_img_info_t level_progress_info; /// 等级进度资源信息
    uint8_t force_update;
    uint8_t popup_ui_index;
    uint8_t display_direction;
    int8_t level_change;
    uint8_t rsv[12];
} level_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_LEVEL_COMP_BG
static PSRAM_DATA_LOC uint8_t setting_bg_data[] = {
    #include "comparison_png2txt/right_level_adjust_360x360.txt"
};
#endif

static level_display_info_t *level_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void pump_level_root_init(void)
{
    if (level_info->root == NULL || !lv_obj_is_valid(level_info->root)) {
        level_info->root = lv_obj_create(lv_screen_active());
        lv_obj_set_size(level_info->root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(level_info->root, lv_color_black(), 0);
        // lv_obj_set_style_bg_color(level_info->root, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(level_info->root, 0, 0);
        lv_obj_set_style_pad_all(level_info->root, 0, 0);
        lv_obj_align(level_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }
}

#if IS_SUPPORT_PUMP_LEVEL_COMP_BG
static void pump_level_root_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);
    
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (level_info->background == NULL || !lv_obj_is_valid(level_info->background)) {
        level_info->background = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->background);
    }
    
    if (status == 0) {
        lv_obj_add_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_path_ex_t bg_img_path;
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, setting_bg_data,
            sizeof(setting_bg_data), "png");
        lv_image_set_src(level_info->background, (const void *)&bg_img_path);
        lv_obj_align(level_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_level_root_bg_update(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    EXCEPTION_CHECK_VOID(level_info->root);
    
    if (level_info->force_update == false) {
        return;
    }

#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = thiz->get_lvgl_snap_shot(UI_SNAP_SHOT_PUMP_LEVEL_BG);
    EXCEPTION_CHECK_VOID(background_snapshot);
#endif

    lv_fs_path_ex_t bg_img_path;
    if (level_info->background == NULL || !lv_obj_is_valid(level_info->background)) {
        level_info->background = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->background);
        lv_obj_align(level_info->background, LV_ALIGN_CENTER, 0, 0);
#if LV_USE_SNAPSHOT
        lv_image_set_src(level_info->background, background_snapshot);
#else
        lv_obj_add_flag(level_info->background, LV_OBJ_FLAG_HIDDEN); /// 隐藏
#endif
    }
}
#endif

static void pump_level_title_text_update(ui_manager_context_t *thiz)
{    
    if (level_info->force_update == false) {
        return;
    }

    if (level_info->title_text == NULL || !lv_obj_is_valid(level_info->title_text)) {
        level_info->title_text = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->title_text);
        lv_label_set_text(level_info->title_text, "Level");
        lv_obj_set_style_text_color(level_info->title_text, lv_color_hex(PUMP_TITLE_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->title_text, &lv_font_oppo_sans_24_medium, 0);
        lv_obj_align(level_info->title_text, LV_ALIGN_TOP_MID, 0, PUMP_LEVEL_TITLE_TEXT_Y);
    }   
}

static void pump_level_direction_update(ui_manager_context_t *thiz, bool is_left)
{    
    if (level_info->force_update == false && level_info->display_direction == is_left) {
        return;
    }

    if (level_info->level_direction == NULL || !lv_obj_is_valid(level_info->level_direction)) {
        level_info->level_direction = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->level_direction);
        lv_obj_set_style_text_color(level_info->level_direction, lv_color_hex(PUMP_LEVEL_DIR_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->level_direction, &lv_font_oppo_sans_24_medium, 0);
        lv_obj_align(level_info->level_direction, LV_ALIGN_TOP_MID, 0, 292 + 6);
    }
    lv_label_set_text(level_info->level_direction, is_left ? "L" : "R");
    level_info->display_direction = is_left;
}

static int pump_level_progress_update(ui_manager_context_t *thiz, uint8_t level)
{   
    if (level_info->level_progress == NULL || !lv_obj_is_valid(level_info->level_progress)) {
        level_info->level_progress = lv_image_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->level_progress);
        lv_obj_align(level_info->level_progress, LV_ALIGN_CENTER, 0, 0);
    }

    if (level > PUMP_LEVEL_MAX || level == 0) {
        lv_obj_add_flag(level_info->level_progress, LV_OBJ_FLAG_HIDDEN);
        return 0;
    }
    lv_obj_clear_flag(level_info->level_progress, LV_OBJ_FLAG_HIDDEN);
    common_ui_img_resource_init(thiz, (RESOURCE_ID_PUMP_LEVEL_START + level - 1), &level_info->level_progress_info);
    EXCEPTION_CHECK_INT(level_info->level_progress_info.data);
    lv_fs_path_ex_t level_progress_path;
    lv_fs_make_path_from_buffer(&level_progress_path, LV_FS_MEMFS_LETTER, 
        level_info->level_progress_info.data, level_info->level_progress_info.size, "baj");
    lv_image_set_src(level_info->level_progress, (const void *)&level_progress_path);
    return 0;
}

static int pump_level_text_update(ui_manager_context_t *thiz, uint8_t level)
{
    if (level_info->level_text == NULL || !lv_obj_is_valid(level_info->level_text)) {
        level_info->level_text = lv_label_create(level_info->root);
        EXCEPTION_CHECK_INT(level_info->level_text);
        lv_obj_set_style_text_color(level_info->level_text, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->level_text, &lv_font_oppo_sans_96_bold, 0);
        lv_obj_set_width(level_info->level_text, 120);
        lv_label_set_long_mode(level_info->level_text, LV_LABEL_LONG_CLIP); /// 截断过长文本
        lv_obj_set_style_text_align(level_info->level_text, LV_TEXT_ALIGN_CENTER, 0); /// 居中对齐
        lv_obj_align(level_info->level_text, LV_ALIGN_CENTER, 0, 0);
    }  
    char level_text[4] = {0};
    snprintf(level_text, sizeof(level_text), "%02d", level);
    lv_label_set_text(level_info->level_text, level_text);
    return 0;
}

static void pump_level_update(ui_manager_context_t *thiz, uint8_t level, int8_t level_change)
{
    if (level_info->force_update == false && level_info->level_change == level_change) {
        return;
    }
     
    if (level_change >= 0) {
        level = level + level_change;
        if (level > PUMP_LEVEL_MAX) {
            level = PUMP_LEVEL_MAX;
        }
    } else {
        uint8_t abs_level_change = abs(level_change);
        level = level < abs_level_change ? 0 : level - abs_level_change;
    }

    if (pump_level_progress_update(thiz, level) != 0) {
        return;
    }

    if (pump_level_text_update(thiz, level) != 0) {
        return;
    }
    level_info->level_change = level_change;
}

static void pump_level_range_text_update(ui_manager_context_t *thiz)
{
    if (level_info->force_update == false) {
        return;
    }

    if (level_info->level_range_text == NULL || !lv_obj_is_valid(level_info->level_range_text)) {
        level_info->level_range_text = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->level_range_text);
        lv_obj_set_style_text_color(level_info->level_range_text, lv_color_hex(PUMP_LEVEL_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(level_info->level_range_text, &lv_font_oppo_sans_24_medium, 0);
        lv_obj_set_style_text_opa(level_info->level_range_text, LV_OPA_40, LV_PART_MAIN);
        lv_obj_set_width(level_info->level_range_text, 120);
        lv_label_set_long_mode(level_info->level_range_text, LV_LABEL_LONG_CLIP); /// 截断过长文本
        lv_obj_set_style_text_align(level_info->level_range_text, LV_TEXT_ALIGN_CENTER, 0); /// 居中对齐
        lv_obj_align(level_info->level_range_text, LV_ALIGN_CENTER, 82, 30);
        lv_label_set_text(level_info->level_range_text, "/15");
    }  
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_level_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (level_display_info_t *)u_malloz(sizeof(level_display_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_level_root_init();
    level_info->force_update = true;
    level_info->popup_ui_index = PUMP_POPUP_UI_NONE;
}

void ui_display_pump_level_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    common_ui_img_resource_deinit(thiz, &level_info->level_progress_info);
    pump_tip_popup_ui_remove(thiz);
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int ui_display_pump_level_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(level_info && level_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump && proc->motor);
    pump_level_root_bg_update(thiz);
    pump_level_title_text_update(thiz);
    if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        pump_level_direction_update(thiz, true);
        pump_level_update(thiz, proc->setting.level[0], proc->level_change);
    } else {
        pump_level_direction_update(thiz, false);
        pump_level_update(thiz, proc->setting.level[1], proc->level_change);
    }
    pump_level_range_text_update(thiz);
    pump_tip_popup_ui_update(thiz, proc, level_info->root, &level_info->popup_ui_index);
    level_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> pump level fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    
    return 0;
}