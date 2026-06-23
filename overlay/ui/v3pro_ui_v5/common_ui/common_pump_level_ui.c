/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

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
#define PUMP_LEVEL_ANIMATION_TIME                             (100)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * title_text; /// 档位文字
    lv_obj_t * level_direction;    /// 等级方向
    lv_obj_t * level_text; /// 等级偏移文字
    lv_obj_t * level_range_text; /// 等级范围文字
    lv_obj_t * level_progress; /// 等级进度
    res_img_info_t level_progress_info; /// 等级进度资源信息
    res_img_info_t direction_info; /// 等级进度资源信息
    uint8_t force_update;
    uint8_t popup_ui_index;
    uint8_t direction;
    uint8_t animation_status;
    uint32_t animation_start_time;
    uint8_t display_direction;
    uint8_t level_base;
    int8_t level_change;
    uint8_t rsv[12];
} level_display_info_t;

typedef enum {
    LEVEL_ANIMATION_TYPE_NONE = 0,
    LEVEL_ANIMATION_TYPE_ZOOM_OUT,
    LEVEL_ANIMATION_TYPE_ZOOM_IN,
} level_animation_type_e;

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
    EXCEPTION_CHECK_VOID(thiz && level_info && level_info->root);
    if (level_info->force_update == false) {
        return;
    }

#if LV_USE_SNAPSHOT
    lv_draw_buf_t *background_snapshot = get_lvgl_snap_shot(UI_SNAP_SHOT_PUMP_LEVEL_BG);
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

static void pump_level_scale_anim_cb(void *obj, int32_t v)
{
    if (obj == NULL || !lv_obj_is_valid((lv_obj_t *)obj)) {
        return;
    }

    lv_image_set_scale((lv_obj_t *)obj, (uint32_t)v);
}

static void pump_level_scale_to(lv_obj_t *img, uint32_t start, uint32_t end, uint32_t time_ms)
{
    if (img == NULL || !lv_obj_is_valid(img)) {
        return;
    }

    lv_anim_del(img, pump_level_scale_anim_cb);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, pump_level_scale_anim_cb);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, time_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

static void pump_level_direction_update(ui_manager_context_t *thiz, bool is_left)
{    
    if (level_info->force_update == false && level_info->display_direction == is_left) {
        level_info->animation_status = LEVEL_ANIMATION_TYPE_NONE;
        level_info->animation_start_time = 0;
        return;
    } 

    if (level_info->level_direction == NULL || !lv_obj_is_valid(level_info->level_direction)) {
        level_info->level_direction = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->level_direction);
        lv_obj_align(level_info->level_direction, LV_ALIGN_TOP_MID, 0, 284);
    }

    if (level_info->force_update == false) {
        if (get_timestamp_ms(level_info->animation_start_time) > (PUMP_LEVEL_ANIMATION_TIME + 50)) {
            if (level_info->animation_status == LEVEL_ANIMATION_TYPE_NONE) {
                level_info->animation_status = LEVEL_ANIMATION_TYPE_ZOOM_OUT;
                common_ui_img_resource_init(thiz, level_info->display_direction > 0 ? \
                    BASE_RES_ID_PUMP_LEFT_LEVEL_ICON : BASE_RES_ID_PUMP_RIGHT_LEVEL_ICON, &level_info->direction_info);
                EXCEPTION_CHECK_VOID(level_info->direction_info.data);
                lv_fs_path_ex_t direction_path;
                lv_fs_make_path_from_buffer(&direction_path, LV_FS_MEMFS_LETTER, level_info->direction_info.data,
                    level_info->direction_info.size, "png");
                lv_image_set_src(level_info->level_direction, (const void *)&direction_path);
                level_info->animation_start_time = get_timestamp();
                pump_level_scale_to(level_info->level_direction, 256, 0, PUMP_LEVEL_ANIMATION_TIME);  // 缩小
            } else if (level_info->animation_status == LEVEL_ANIMATION_TYPE_ZOOM_OUT) {
                common_ui_img_resource_init(thiz, is_left > 0 ? \
                    BASE_RES_ID_PUMP_LEFT_LEVEL_ICON : BASE_RES_ID_PUMP_RIGHT_LEVEL_ICON, &level_info->direction_info);
                EXCEPTION_CHECK_VOID(level_info->direction_info.data);
                lv_fs_path_ex_t direction_path;
                lv_fs_make_path_from_buffer(&direction_path, LV_FS_MEMFS_LETTER, level_info->direction_info.data,
                    level_info->direction_info.size, "png");
                lv_image_set_src(level_info->level_direction, (const void *)&direction_path);
                level_info->animation_status = LEVEL_ANIMATION_TYPE_ZOOM_IN;
                level_info->animation_start_time = get_timestamp();
                pump_level_scale_to(level_info->level_direction, 0, 256, PUMP_LEVEL_ANIMATION_TIME);  // 放大
            } else {
                level_info->animation_status = LEVEL_ANIMATION_TYPE_NONE;
                level_info->display_direction = is_left;
            }
        }
    } else {
        common_ui_img_resource_init(thiz, is_left > 0 ? \
            BASE_RES_ID_PUMP_LEFT_LEVEL_ICON : BASE_RES_ID_PUMP_RIGHT_LEVEL_ICON, &level_info->direction_info);
        EXCEPTION_CHECK_VOID(level_info->direction_info.data);
        lv_fs_path_ex_t direction_path;
        lv_fs_make_path_from_buffer(&direction_path, LV_FS_MEMFS_LETTER, level_info->direction_info.data,
            level_info->direction_info.size, "png");
        lv_image_set_src(level_info->level_direction, (const void *)&direction_path);
        level_info->animation_status = LEVEL_ANIMATION_TYPE_NONE;
        level_info->display_direction = is_left;
    }
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
    common_ui_img_resource_init(thiz, (BASE_RES_ID_PUMP_LEVEL_START + level - 1), &level_info->level_progress_info);
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
    if (level_info->force_update == false && level_info->level_change == level_change &&
        level_info->level_base == level) {
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
    level_info->level_base = level;
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
void common_pump_level_ui_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (level_display_info_t *)u_malloz(sizeof(level_display_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    pump_level_root_init();
    level_info->force_update = true;
    level_info->popup_ui_index = ID_POPUP_UI_NONE;
    level_info->animation_status = LEVEL_ANIMATION_TYPE_NONE;
    level_info->animation_start_time = 0;
}

void common_pump_level_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(level_info);
    common_ui_img_resource_deinit(thiz, &level_info->level_progress_info);
    common_ui_img_resource_deinit(thiz, &level_info->direction_info);
    ui_common_operation_tip_remove(thiz);
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int common_pump_level_ui_process(ui_manager_context_t *thiz, uint8_t level, 
    int8_t level_change, bool is_left, uint8_t popup_ui_type)
{
    EXCEPTION_CHECK_INT(thiz && level_info && level_info->root);
    pump_level_root_bg_update(thiz);
    pump_level_title_text_update(thiz);
    pump_level_direction_update(thiz, is_left);
    pump_level_update(thiz, level, level_change);
    pump_level_range_text_update(thiz);
    ui_common_operation_tip_update(thiz, popup_ui_type, level_info->root, &level_info->popup_ui_index);
    level_info->force_update = false;
        
    return 0;
}