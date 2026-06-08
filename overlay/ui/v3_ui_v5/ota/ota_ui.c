/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_OTA_COMP_BG                      (0)  
#define OTA_DESC_TEXT_NUM                           (2)   
#define OTA_PROGRESS_LINE_WIDTH                     (8)
#define OTA_PROGRESS_LINE_LENGHT                    (264)
#define OTA_PROGRESS_TEXT_COLOR                     (0x4F4D4D)
#define OTA_RESULT_TEXT_COLOR                       (0x767676)
#define OTA_TITL_TEXT_COLOR                         (0xFFFFFF)
#define OTA_PROGRESS_BAR_COLOR                      (0xB25962)
#define OTA_PROGRESS_BG_COLOR                       (0x4A4A4A)

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_OTA_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * progress_bg;
    lv_obj_t * progress_bar;
    lv_obj_t * ota_desc[2];
    lv_obj_t * ota_percent;
    lv_obj_t * ota_size;
    lv_obj_t * icon;
    res_img_info_t icon_info;
    uint32_t start_time;
    uint32_t display_time;
    uint32_t total_size;
    uint32_t recv_size;
    uint8_t force_update;
    uint8_t status;
    uint8_t progress;
    uint8_t object;
    uint8_t rsv[12];
} ota_display_info_t;

typedef struct {
    const char *name;
    const lv_font_t *font;
    uint32_t color;
    uint16_t y_offset;
} ota_text_info_t;

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

const static ota_text_info_t ota_res_downloading_text_infos[OTA_DESC_TEXT_NUM] = {
    {"Theme setting", 
        &lv_font_oppo_sans_20_medium, OTA_TITL_TEXT_COLOR, 126},
    {"Restore factory settings to reselect", 
        &lv_font_oppo_sans_16_medium, OTA_RESULT_TEXT_COLOR, 158}
};

const static ota_text_info_t ota_res_error_text_infos[OTA_DESC_TEXT_NUM] = {
    {"Theme update failed", 
        &lv_font_oppo_sans_20_medium, OTA_RESULT_TEXT_COLOR, 212},
    {"", 
        &lv_font_oppo_sans_16_medium, 0, 0}
};

const static ota_text_info_t ota_res_suceess_text_infos[OTA_DESC_TEXT_NUM] = {
    {"Theme updated", 
        &lv_font_oppo_sans_20_medium, OTA_RESULT_TEXT_COLOR, 212},
    {"", 
        &lv_font_oppo_sans_16_medium, 0, 0}
};

const static ota_text_info_t ota_app_downloading_text_infos[OTA_DESC_TEXT_NUM] = {
    {"Firmware upgrading", 
        &lv_font_oppo_sans_20_medium, OTA_TITL_TEXT_COLOR, 138},
    {"", 
        &lv_font_oppo_sans_16_medium, 0, 0}
};

const static ota_text_info_t ota_hw_downloading_text_infos[OTA_DESC_TEXT_NUM] = {
    {"Hardware upgrading", 
        &lv_font_oppo_sans_20_medium, OTA_TITL_TEXT_COLOR, 138},
    {"", 
        &lv_font_oppo_sans_16_medium, 0, 0}
};

const static ota_text_info_t ota_error_text_infos[OTA_DESC_TEXT_NUM] = {
    {"Upgrade failed", 
        &lv_font_oppo_sans_20_medium, OTA_RESULT_TEXT_COLOR, 212},
    {"", 
        &lv_font_oppo_sans_16_medium, 0, 0}
};

const static ota_text_info_t ota_suceess_text_infos[OTA_DESC_TEXT_NUM] = {
    {"Upgrade completed", 
        &lv_font_oppo_sans_20_medium, OTA_RESULT_TEXT_COLOR, 212},
    {"", 
        &lv_font_oppo_sans_16_medium, 0, 0}
};

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

static void ui_ota_progress_arc_update(ui_manager_context_t *thiz, uint8_t progress, bool is_show, 
    uint16_t y_offset)
{
    if (progress > 100) {
        progress = 100;
    }

    if (ota_info->force_update == false && ota_info->progress == progress) {
        return;
    }

    if (ota_info->progress_bg == NULL || !lv_obj_is_valid(ota_info->progress_bg)) {
        ota_info->progress_bg = lv_bar_create(ota_info->root); 
        EXCEPTION_CHECK_VOID(ota_info->progress_bg);
        lv_obj_set_size(ota_info->progress_bg, OTA_PROGRESS_LINE_LENGHT, OTA_PROGRESS_LINE_WIDTH);
        lv_obj_set_style_radius(ota_info->progress_bg, OTA_PROGRESS_LINE_WIDTH / 2, 0);
        lv_obj_set_style_border_width(ota_info->progress_bg, 0, 0);
        lv_obj_set_style_pad_all(ota_info->progress_bg, 0, 0);
        lv_obj_set_style_bg_color(ota_info->progress_bg, lv_color_hex(OTA_PROGRESS_BG_COLOR), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ota_info->progress_bg, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_obj_align(ota_info->progress_bg, LV_ALIGN_TOP_MID, 0, y_offset + 24);
    }

    if (ota_info->progress_bar == NULL || !lv_obj_is_valid(ota_info->progress_bar)) {
        ota_info->progress_bar = lv_bar_create(ota_info->progress_bg);
        EXCEPTION_CHECK_VOID(ota_info->progress_bar);        
        lv_obj_set_size(ota_info->progress_bar, OTA_PROGRESS_LINE_LENGHT, OTA_PROGRESS_LINE_WIDTH);
        lv_obj_align(ota_info->progress_bar, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(ota_info->progress_bar, OTA_PROGRESS_LINE_WIDTH / 2, 0);
        lv_obj_set_style_border_width(ota_info->progress_bar, 0, 0);
        lv_obj_set_style_pad_all(ota_info->progress_bar, 0, 0);
        lv_obj_set_style_bg_opa(ota_info->progress_bar, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_color(ota_info->progress_bar, lv_color_hex(OTA_PROGRESS_BAR_COLOR), LV_PART_INDICATOR);
        lv_obj_set_style_radius(ota_info->progress_bar, OTA_PROGRESS_LINE_WIDTH / 2, LV_PART_INDICATOR);
    }

    if (ota_info->ota_percent == NULL || !lv_obj_is_valid(ota_info->ota_percent)) {
        ota_info->ota_percent = lv_label_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->ota_percent);
        lv_obj_set_style_text_font(ota_info->ota_percent, &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_color(ota_info->ota_percent, lv_color_hex(OTA_PROGRESS_TEXT_COLOR), 0);
        lv_obj_align(ota_info->ota_percent, LV_ALIGN_TOP_LEFT, 277, y_offset);
        lv_obj_set_style_text_align(ota_info->ota_percent, LV_TEXT_ALIGN_LEFT, 0);
    }

    if (!is_show) {
        lv_obj_add_flag(ota_info->progress_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ota_info->progress_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ota_info->ota_percent, LV_OBJ_FLAG_HIDDEN);
        return;
    } else {
        lv_obj_clear_flag(ota_info->progress_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ota_info->progress_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ota_info->ota_percent, LV_OBJ_FLAG_HIDDEN);
    }
    lv_bar_set_value(ota_info->progress_bar, progress, LV_ANIM_OFF);  
    char ota_percent[8] = {0};
    snprintf(ota_percent, sizeof(ota_percent), "%d%%", progress);
    lv_label_set_text(ota_info->ota_percent, ota_percent);
    ota_info->progress = progress;
}

static void ui_ota_icon_update(ui_manager_context_t *thiz, uint16_t img_id, bool is_show)
{
    if (ota_info->force_update == false) {
        return;
    }

    if (ota_info->icon == NULL || !lv_obj_is_valid(ota_info->icon)) {
        ota_info->icon = lv_image_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->icon);
        lv_obj_align(ota_info->icon, LV_ALIGN_TOP_MID, 0, 124); /// 设置icon位置
    }

    if (!is_show) {
        lv_obj_add_flag(ota_info->icon, LV_OBJ_FLAG_HIDDEN);
        return;
    } else {
        lv_obj_clear_flag(ota_info->icon, LV_OBJ_FLAG_HIDDEN);
    }
    common_ui_img_resource_init(thiz, img_id, &ota_info->icon_info);
    EXCEPTION_CHECK_VOID(ota_info->icon_info.data);
    lv_fs_path_ex_t ota_icon_img_path;
    lv_fs_make_path_from_buffer(&ota_icon_img_path, LV_FS_MEMFS_LETTER, ota_info->icon_info.data, ota_info->icon_info.size, "baj");
    lv_image_set_src(ota_info->icon, (const void *)&ota_icon_img_path);
}

static void ui_ota_desc_update(ui_manager_context_t *thiz, const ota_text_info_t *text_info)
{
    EXCEPTION_CHECK_VOID(text_info);
    if (ota_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < OTA_DESC_TEXT_NUM; i++) {
        if (ota_info->ota_desc[i] == NULL || !lv_obj_is_valid(ota_info->ota_desc[i])) {
            ota_info->ota_desc[i] = lv_label_create(ota_info->root);
            EXCEPTION_CHECK_VOID(ota_info->ota_desc[i]);
        }
        lv_label_set_text(ota_info->ota_desc[i], text_info[i].name);
        lv_obj_set_style_text_color(ota_info->ota_desc[i], lv_color_hex(text_info[i].color), 0);
        lv_obj_set_style_text_font(ota_info->ota_desc[i], text_info[i].font, 0);  /// 设置字体
        lv_obj_set_style_text_line_space(ota_info->ota_desc[i], 4, 0);  /// 设置行间距（像素）
        lv_obj_set_style_text_align(ota_info->ota_desc[i], LV_TEXT_ALIGN_CENTER, 0);  /// 设置文本居中对齐
        lv_obj_align(ota_info->ota_desc[i], LV_ALIGN_TOP_MID, 0, text_info[i].y_offset);
    }
}

static void ui_ota_size_update(ui_manager_context_t *thiz, uint32_t total_size, uint32_t recv_size, 
    uint8_t is_show, uint16_t y_offset)
{
    if (ota_info->force_update == false && ota_info->recv_size == recv_size && ota_info->total_size == total_size) {
        return;
    }

    if (ota_info->ota_size == NULL || !lv_obj_is_valid(ota_info->ota_size)) {
        ota_info->ota_size = lv_label_create(ota_info->root);
        EXCEPTION_CHECK_VOID(ota_info->ota_size);
        lv_obj_set_style_text_font(ota_info->ota_size, &lv_font_oppo_sans_16_medium, 0);
        lv_obj_set_style_text_color(ota_info->ota_size, lv_color_hex(OTA_PROGRESS_TEXT_COLOR), 0);
        lv_obj_align(ota_info->ota_size, LV_ALIGN_TOP_LEFT, 48, y_offset); /// 设置文本位置
        lv_obj_set_style_text_align(ota_info->ota_size, LV_TEXT_ALIGN_LEFT, 0);  /// 设置文本居中对齐
    }

    if (!is_show) {
        lv_obj_add_flag(ota_info->ota_size, LV_OBJ_FLAG_HIDDEN);
        return;
    } else {
        lv_obj_clear_flag(ota_info->ota_size, LV_OBJ_FLAG_HIDDEN);
    }
    char ota_percent[32] = {0};
    if (total_size >= 1048576) {
        float total_f = total_size / 1048576.0f;
        if (recv_size >= 1048576) {
            float recv_f = recv_size / 1048576.0f;
            snprintf(ota_percent, sizeof(ota_percent), "%0.1fMB/%0.1fMB", recv_f, total_f);
        } else {
            float recv_f = recv_size / 1024.0f;
            snprintf(ota_percent, sizeof(ota_percent), "%0.1fKB/%0.1fMB", recv_f, total_f);
        }
    } else {
        float recv_f = recv_size / 1024.0f;
        float total_f = total_size / 1024.0f;
        snprintf(ota_percent, sizeof(ota_percent), "%0.1fKB/%0.1fKB", recv_f, total_f);
    }
    LOG_UI("ota progress %s, %d,%d", ota_percent, recv_size, total_size);
    lv_label_set_text(ota_info->ota_size, ota_percent);
    ota_info->recv_size = recv_size;
    ota_info->total_size = total_size;
}

static void ui_ota_status_update(ui_manager_context_t *thiz, ota_info_t info, uint8_t ota_status)
{
    if (ota_info->status != ota_status || ota_info->object != info.object) {
        ota_info->status = ota_status;
        ota_info->object = info.object;
        ota_info->force_update = true;
    }

    switch(ota_status) {
        default:
        case OTA_UI_STATUS_DOWNLOADING:
            ui_ota_icon_update(thiz, 0, false);
            if (info.object == OTA_OBJECT_MAIN_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_app_downloading_text_infos);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, true, 188);
                ui_ota_progress_arc_update(thiz, info.progress, true, 188);
            } else if (info.object == OTA_OBJECT_IMAGE_RESOURCE) {
                ui_ota_desc_update(thiz, ota_res_downloading_text_infos);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, true, 198);
                ui_ota_progress_arc_update(thiz, info.progress, true, 198);
            } else if (info.object == OTA_OBJECT_TOUCH_KEY_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_hw_downloading_text_infos);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, true, 188);
                ui_ota_progress_arc_update(thiz, info.progress, true, 188);
            }
            break;
        case OTA_UI_STATUS_ERROR:
            ui_ota_icon_update(thiz, RESOURCE_ID_BAN_ICON, true);
            if (info.object == OTA_OBJECT_MAIN_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_error_text_infos);
                ui_ota_progress_arc_update(thiz, info.progress, false, 188);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false, 188);
            } else if (info.object == OTA_OBJECT_IMAGE_RESOURCE) {
                ui_ota_desc_update(thiz, ota_res_error_text_infos);
                ui_ota_progress_arc_update(thiz, info.progress, false, 198);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false, 198);
            } else if (info.object == OTA_OBJECT_TOUCH_KEY_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_error_text_infos);
                ui_ota_progress_arc_update(thiz, info.progress, false, 188);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false, 188);
            }
            break;
        case OTA_UI_STATUS_COMPLETE:
            ui_ota_icon_update(thiz, RESOURCE_ID_ALLOW_ICON, true);
            if (info.object == OTA_OBJECT_MAIN_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_suceess_text_infos);
                ui_ota_progress_arc_update(thiz, info.progress, false, 188);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false, 188);
            } else if (info.object == OTA_OBJECT_IMAGE_RESOURCE) {
                ui_ota_desc_update(thiz, ota_res_suceess_text_infos);
                ui_ota_progress_arc_update(thiz, info.progress, false, 198);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false, 198);
            } else if (info.object == OTA_OBJECT_TOUCH_KEY_FIRMWARE) {
                ui_ota_desc_update(thiz, ota_suceess_text_infos);
                ui_ota_progress_arc_update(thiz, info.progress, false, 188);
                ui_ota_size_update(thiz, info.total_size, info.recv_size, false, 188);
            }
            break;
    }
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
    ota_info_t ota_dev_info = {0};
    proc->ota->get_ota_info(&ota_dev_info); 
#if IS_SUPPORT_OTA_COMP_BG
    ui_ota_background_update(thiz);
#endif
    ui_ota_status_update(thiz, ota_dev_info, proc->ota_status);
    ota_info->force_update = false;
    thiz->priv.fps_count++;
    if (proc->ota_status == OTA_UI_STATUS_DOWNLOADING) {
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
