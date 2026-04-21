/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ota_ui.h"
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_OTA_COMP_BG                      (0)

#define OTA_PROGRESS_ARC_SIZE                       (38)    
#define OTA_PROGRESS_ARC_WIDTH                      (3)    
#define OTA_PROGRESS_ARC_BG_COLOR                   (0xFFFFFF)    
#define OTA_PROGRESS_ARC_BG_OPA                     (38)        
#define OTA_PROGRESS_ARC_COLOR                      (0xB25962)    
#define OTA_PROGRESS_ARC_LOOP_MS_PER_DEG            (3)    
#define OTA_PROGRESS_ARC_ANGLE_SPAN                 (90)   
#define OTA_PROGRESS_ARC_TEXT_COLOR                 (0x767676)    

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_OTA_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * progress_arc;
    lv_obj_t * status_text;
    lv_obj_t * icon;
    res_img_info_t icon_info;
    uint32_t start_time;
    uint32_t display_time;
    uint8_t force_update;
    uint8_t ota_status;
    uint8_t progress;
    uint8_t ota_object;
    uint8_t rsv[12];
    char text[64];
} ota_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ota_display_info_t *ota_info = NULL;
#if IS_SUPPORT_OTA_COMP_BG
static PSRAM_DATA_LOC uint8_t factory_reset_confirm_bg_data[] = {
    #include "comparison_png2txt/ota_running_360x360.txt"
};
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_ota_root_init(void)
{
    if (ota_info->root == NULL || !lv_obj_is_valid(ota_info->root)) {
        ota_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(ota_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(ota_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(ota_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(ota_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(ota_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(ota_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ota_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(ota_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void ui_ota_progress_arc_update(ui_manager_context_t *thiz, uint8_t ota_status)
{
    if (ota_info->progress_arc == NULL || !lv_obj_is_valid(ota_info->progress_arc)) {
        ota_info->progress_arc = lv_arc_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->progress_arc);
    }
    
    if (ota_status == OTA_UI_STATUS_DOWNLOADING) {
        lv_obj_clear_flag(ota_info->progress_arc, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ota_info->progress_arc, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_set_size(ota_info->progress_arc, OTA_PROGRESS_ARC_SIZE, OTA_PROGRESS_ARC_SIZE);
    lv_obj_center(ota_info->progress_arc);
    lv_arc_set_bg_angles(ota_info->progress_arc, 0, 360);
    lv_obj_set_style_arc_color(ota_info->progress_arc, lv_color_hex(OTA_PROGRESS_ARC_BG_COLOR), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ota_info->progress_arc, OTA_PROGRESS_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ota_info->progress_arc, OTA_PROGRESS_ARC_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ota_info->progress_arc, lv_color_hex(OTA_PROGRESS_ARC_COLOR), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ota_info->progress_arc, OTA_PROGRESS_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ota_info->progress_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_align(ota_info->progress_arc, LV_ALIGN_TOP_MID, 0, 138);
    lv_obj_remove_style(ota_info->progress_arc, NULL, LV_PART_KNOB);
    if (ota_info->start_time == 0) {
        ota_info->start_time = get_timestamp();
    }
    uint32_t elapsed = get_timestamp() - ota_info->start_time;
    uint16_t start_angle = (uint16_t)((elapsed / OTA_PROGRESS_ARC_LOOP_MS_PER_DEG) % 360);
    uint16_t end_angle = start_angle + OTA_PROGRESS_ARC_ANGLE_SPAN;
    lv_arc_set_angles(ota_info->progress_arc, start_angle, end_angle);
    lv_obj_invalidate(ota_info->progress_arc); /// 使圆弧/图片区域重绘
}

static const char *get_ota_object_name(ota_object_e ota_object)
{
    if (ota_object == OTA_OBJECT_MAIN_FIRMWARE) {
        return "main firmware";
    } else if (ota_object == OTA_OBJECT_TOUCH_KEY_FIRMWARE) {
        return "touch firmware";
    } else if (ota_object == OTA_OBJECT_IMAGE_RESOURCE) {
        return "image resource";
    }
    return "unknown";
}

static void ui_ota_text_update(ui_manager_context_t *thiz, uint8_t ota_status, uint8_t progress)
{
    if (ota_info->force_update == false && ota_info->ota_status == ota_status && 
        ota_info->progress == progress) {
        return;
    }

    if (ota_info->status_text == NULL || !lv_obj_is_valid(ota_info->status_text)) {
        ota_info->status_text = lv_label_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->status_text);
        lv_obj_set_style_text_font(ota_info->status_text, &lv_font_oppo_sans_20_medium, 0);
        lv_obj_set_style_text_color(ota_info->status_text, lv_color_hex(OTA_PROGRESS_ARC_TEXT_COLOR), 0);
        lv_obj_align(ota_info->status_text, LV_ALIGN_TOP_MID, 0, 210); /// 设置文本位置
        lv_obj_set_style_text_line_space(ota_info->status_text, 4, 0);  /// 设置行间距（像素）
        lv_obj_set_style_text_align(ota_info->status_text, LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
    }
    const char *ota_object_name = get_ota_object_name(ota_info->ota_object);
    memset(ota_info->text, 0, sizeof(ota_info->text));
    if (ota_status == OTA_UI_STATUS_DOWNLOADING) {
        if (ota_info->ota_object == OTA_OBJECT_MAIN_FIRMWARE) {
            snprintf(ota_info->text, sizeof(ota_info->text), "%s\nupgrade progress:%d%%", ota_object_name, progress);
        } else if (ota_info->ota_object == OTA_OBJECT_TOUCH_KEY_FIRMWARE) {
            snprintf(ota_info->text, sizeof(ota_info->text), "%s\nupgrade progress:%d%%", ota_object_name, progress);
        } else if (ota_info->ota_object == OTA_OBJECT_IMAGE_RESOURCE) {
            snprintf(ota_info->text, sizeof(ota_info->text), "%s\nupgrade progress:%d%%", ota_object_name, progress);
        }
        lv_label_set_text(ota_info->status_text, ota_info->text);
    } else if (ota_status == OTA_UI_STATUS_ERROR) {
        snprintf(ota_info->text, sizeof(ota_info->text), "%s\nupgrade failed", ota_object_name);
        lv_label_set_text(ota_info->status_text, ota_info->text);
    } else if (ota_status == OTA_UI_STATUS_COMPLETE) {
        snprintf(ota_info->text, sizeof(ota_info->text), "%s\nupgrade complete", ota_object_name);
        lv_label_set_text(ota_info->status_text, ota_info->text);
    }
}

static void ui_ota_icon_update(ui_manager_context_t *thiz, uint8_t ota_status)
{
    if (ota_info->force_update == false && ota_info->ota_status == ota_status) {
        return;
    }

    if (ota_info->icon == NULL || !lv_obj_is_valid(ota_info->icon)) {
        ota_info->icon = lv_image_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->icon);
        lv_obj_align(ota_info->icon, LV_ALIGN_TOP_MID, 0, 126); /// 设置icon位置
    }

    if (ota_status == OTA_UI_STATUS_DOWNLOADING) {
        lv_obj_add_flag(ota_info->icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ota_info->icon, LV_OBJ_FLAG_HIDDEN);
    }
    common_ui_img_resource_init(thiz, ota_status == OTA_UI_STATUS_COMPLETE ? RESOURCE_ID_ALLOW_ICON : RESOURCE_ID_BAN_ICON, &ota_info->icon_info);
    EXCEPTION_CHECK_VOID(ota_info->icon_info.data);
    lv_fs_path_ex_t ota_icon_img_path;
    lv_fs_make_path_from_buffer(&ota_icon_img_path, LV_FS_MEMFS_LETTER, ota_info->icon_info.data, ota_info->icon_info.size, "baj");
    lv_image_set_src(ota_info->icon, (const void *)&ota_icon_img_path);
}

#if IS_SUPPORT_OTA_COMP_BG
static void ui_ota_background_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (ota_info->background == NULL || !lv_obj_is_valid(ota_info->background)) {
        ota_info->background = lv_image_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(ota_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ota_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, factory_reset_confirm_bg_data,
            sizeof(factory_reset_confirm_bg_data), "png");
        lv_image_set_src(ota_info->background, (const void *)&bg_img_path);
        lv_obj_align(ota_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_ota_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ota_info == NULL) {
        ota_info = (ota_display_info_t *)u_malloz(sizeof(ota_display_info_t));
        EXCEPTION_CHECK_VOID(ota_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_ota_root_init(); /// 初始化 root 容器
    ota_info->force_update = true;
    ota_info->progress = 0;
    ui_proc_ota_t *proc = (ui_proc_ota_t *)thiz->priv.play_info.user_data;
    if (proc && proc->ota) {
        ota_info->ota_object = proc->ota->get_ota_object();
    }
}

void ui_display_ota_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && ota_info && ota_info->root); 
    common_ui_img_resource_deinit(thiz, &ota_info->icon_info);
    LVGL_OBJ_DELETE(ota_info->root);
    u_free(ota_info);
    ota_info = NULL;
}

int ui_display_ota_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && ota_info && ota_info->root);
    ui_proc_ota_t *proc = (ui_proc_ota_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    uint8_t ota_status = proc->ota_status;
    uint8_t progress = proc->ota->get_ota_upgrade_progress();
#if IS_SUPPORT_OTA_COMP_BG
    ui_ota_background_update(thiz);
#endif
    ui_ota_progress_arc_update(thiz, ota_status);
    ui_ota_text_update(thiz, ota_status, progress);
    ui_ota_icon_update(thiz, ota_status);
    ota_info->ota_status = ota_status;
    ota_info->progress = progress;
    ota_info->force_update = false;
    thiz->priv.fps_count++;
    if (ota_status == OTA_UI_STATUS_DOWNLOADING) {
        ota_info->display_time = get_timestamp();
    } else {
        if (get_timestamp_ms(ota_info->display_time) > OTA_RESULT_DISPLAY_TIME) {
            ui_animation_play_t *ani = &thiz->priv.play_info;
            EXCEPTION_CHECK_INT(ani);
            LOG_UI("ota result display timeout");
            ani->progress_cb(thiz, 0, ANI_TIMER_DRAW_FINISH);
        }
    }

    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> ota fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    
    return 0;
}
