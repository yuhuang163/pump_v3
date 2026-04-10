/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h"   // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_STATUS_COMP_BG                        (0)
#define PUMP_STATUS_TEXT_COUNT                                2
#define PUMP_STATUS_TEXT_COLOR                                0xFFFFFF
#define PUMP_STATUS_ARC_SIZE                                  (360)    // 圆弧尺寸
#define PUMP_STATUS_ARC_WIDTH                                 (12)      // 圆弧粗细
#define PUMP_STATUS_ARC_COLOR                                 (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define PUMP_STATUS_ARC_START_ANGLE                           (270)    
#define PUMP_STATUS_ARC_FULL_SWEEP                            (360)    
#define PUMP_STATUS_DISPLAY_STEP_TIME                         (100)
#define PUMP_PAUSE_ICON_Y                                     (118)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * text[PUMP_STATUS_TEXT_COUNT];
    lv_obj_t * arc;                    /// 圆弧进度条对象
    lv_obj_t * mask_layer;             /// 遮罩层对象
    lv_obj_t * status_icon;
    res_img_info_t status_img_info;
    uint32_t start_time;               /// 倒计时起始时间
    uint16_t popup_type;   /// 弹窗类型
    uint16_t ring_progress;            /// 圆弧进度
    uint16_t ring_progress_max;       /// 圆弧进度最大值
    uint16_t target_time;             /// 倒计时目标时间 单位：秒
    uint8_t force_update;
    uint8_t rsv[15];
} pump_status_display_info_t;

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint16_t text_opa; /// 文本不透明度
} pump_status_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_status_display_info_t *pump_status_info;

const static pump_status_text_info_t pump_pause_manual_infos[PUMP_STATUS_TEXT_COUNT] = {
    { "Pumping paused", 199, &lv_font_oppo_sans_16_medium, LV_OPA_100},
    { "Auto shutoff after 15 minutes of inactivity", 226, &lv_font_oppo_sans_14_medium, LV_OPA_70},
};

const static pump_status_text_info_t pump_pause_timeout_infos[PUMP_STATUS_TEXT_COUNT] = {
    { "Pumping over 30 min", 199, &lv_font_oppo_sans_16_medium, LV_OPA_100},
    { "Take a short break", 226, &lv_font_oppo_sans_14_medium, LV_OPA_70},
};

const static pump_status_text_info_t pump_finish_manual_infos[PUMP_STATUS_TEXT_COUNT] = {
    { "Ending", 180, &lv_font_oppo_sans_20_medium, LV_OPA_100},
    { "", 226, &lv_font_oppo_sans_16_medium, LV_OPA_70},
};

const static pump_status_text_info_t pump_finish_timeout_infos[PUMP_STATUS_TEXT_COUNT] = {
    { "Pumping timeout", 159, &lv_font_oppo_sans_20_medium, LV_OPA_100},
    { "Ending automatically", 190, &lv_font_oppo_sans_16_medium, LV_OPA_70},
};

#if IS_SUPPORT_PUMP_STATUS_COMP_BG
static PSRAM_DATA_LOC uint8_t pump_status_comp_bg_data[] = {
    #include "comparison_png2txt/pump_over_30min_popup_360x360.txt"
    // #include "comparison_png2txt/pump_pause_popup_360x360.txt"
    // #include "comparison_png2txt/pumping_timeout_360x360.txt"
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
static void pump_status_root_init(lv_obj_t *parent)
{
    if (pump_status_info->root == NULL || !lv_obj_is_valid(pump_status_info->root)) {
        pump_status_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(pump_status_info->root);
    }
    lv_obj_set_size(pump_status_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(pump_status_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(pump_status_info->root, LV_OPA_TRANSP, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(pump_status_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(pump_status_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(pump_status_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(pump_status_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(pump_status_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void pump_status_icon_update(ui_manager_context_t *thiz, uint16_t img_id)
{
    if (pump_status_info->force_update == false) {
        return;
    }
    
    if (pump_status_info->status_icon == NULL || !lv_obj_is_valid(pump_status_info->status_icon)) {
        pump_status_info->status_icon = lv_image_create(pump_status_info->root);
        EXCEPTION_CHECK_VOID(pump_status_info->status_icon);
        lv_obj_align(pump_status_info->status_icon, LV_ALIGN_TOP_MID, 0, PUMP_PAUSE_ICON_Y);
    }

    common_ui_img_resource_deinit(thiz, &pump_status_info->status_img_info);
    common_ui_img_resource_init(thiz, img_id, &pump_status_info->status_img_info);
    EXCEPTION_CHECK_VOID(pump_status_info->status_img_info.data);
    lv_fs_path_ex_t status_img_path;
    lv_fs_make_path_from_buffer(&status_img_path, LV_FS_MEMFS_LETTER, 
        pump_status_info->status_img_info.data, pump_status_info->status_img_info.size, "baj");
    lv_image_set_src(pump_status_info->status_icon, (const void *)&status_img_path);
}

static void pump_status_arc_update(ui_manager_context_t *thiz)
{
    uint16_t ring_progress = get_timestamp_ms(pump_status_info->start_time) / PUMP_STATUS_DISPLAY_STEP_TIME;
    if (pump_status_info->force_update == false && pump_status_info->ring_progress == ring_progress) {
        return;
    }
    pump_status_info->ring_progress = ring_progress;
    if (pump_status_info->arc == NULL || !lv_obj_is_valid(pump_status_info->arc)) {
        pump_status_info->arc = lv_arc_create(pump_status_info->root);
        EXCEPTION_CHECK_VOID(pump_status_info->arc);
        lv_obj_set_size(pump_status_info->arc, PUMP_STATUS_ARC_SIZE, PUMP_STATUS_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(pump_status_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(pump_status_info->arc, 0, PUMP_STATUS_ARC_FULL_SWEEP);  /// 背景弧为完整闭环
        lv_arc_set_angles(pump_status_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(pump_status_info->arc, PUMP_STATUS_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(pump_status_info->arc, lv_color_hex(PUMP_STATUS_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(pump_status_info->arc, PUMP_STATUS_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(pump_status_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(pump_status_info->arc, lv_color_hex(PUMP_STATUS_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(pump_status_info->arc, PUMP_STATUS_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(pump_status_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(pump_status_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (ring_progress >= pump_status_info->ring_progress_max) {
        lv_obj_set_style_arc_opa(pump_status_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(pump_status_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(pump_status_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(pump_status_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(pump_status_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (PUMP_STATUS_ARC_FULL_SWEEP * ring_progress) / pump_status_info->ring_progress_max;
    }
    lv_arc_set_angles(pump_status_info->arc, 0, end_angle);
}

static void pump_status_text_update(ui_manager_context_t *thiz, const pump_status_text_info_t *text_info)
{
    if (pump_status_info->force_update == false) {
        return;
    }
    EXCEPTION_CHECK_VOID(text_info);
    for (uint8_t i = 0; i < PUMP_STATUS_TEXT_COUNT; i++) {
        if (pump_status_info->text[i] == NULL || !lv_obj_is_valid(pump_status_info->text[i])) {
            pump_status_info->text[i] = lv_label_create(pump_status_info->root);
            EXCEPTION_CHECK_VOID(pump_status_info->text[i]);
            lv_label_set_text(pump_status_info->text[i], text_info[i].name);
            lv_obj_set_style_text_color(pump_status_info->text[i], lv_color_hex(PUMP_STATUS_TEXT_COLOR), 0);
            lv_obj_set_style_text_font(pump_status_info->text[i], text_info[i].font, 0);
            lv_obj_set_style_text_opa(pump_status_info->text[i], text_info[i].text_opa, 0);
            lv_obj_align(pump_status_info->text[i], LV_ALIGN_TOP_MID, 0, text_info[i].text_y);
        }
    }
}

#if IS_SUPPORT_PUMP_STATUS_COMP_BG
static void pump_status_mask_layer_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (pump_status_info->mask_layer == NULL || !lv_obj_is_valid(pump_status_info->mask_layer)) {
        pump_status_info->mask_layer = lv_image_create(pump_status_info->root);
        EXCEPTION_CHECK_VOID(pump_status_info->mask_layer);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(pump_status_info->mask_layer, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(pump_status_info->mask_layer, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, pump_status_comp_bg_data,
            sizeof(pump_status_comp_bg_data), "png");
        lv_image_set_src(pump_status_info->mask_layer, (const void *)&bg_img_path);
        lv_obj_align(pump_status_info->mask_layer, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 500) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_status_mask_layer_update(ui_manager_context_t *thiz)
{
    if (pump_status_info->force_update == false) {
        return;
    }
 
    if (pump_status_info->mask_layer == NULL || !lv_obj_is_valid(pump_status_info->mask_layer)) {
        pump_status_info->mask_layer = lv_obj_create(pump_status_info->root);
        EXCEPTION_CHECK_VOID(pump_status_info->mask_layer);
        lv_obj_set_size(pump_status_info->mask_layer, LV_PCT(100), LV_PCT(100));
        lv_obj_center(pump_status_info->mask_layer);
        lv_obj_set_style_bg_color(pump_status_info->mask_layer, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(pump_status_info->mask_layer, 190, 0); /// 黑色背景透明度190
        lv_obj_set_style_border_width(pump_status_info->mask_layer, 0, 0);
        lv_obj_set_style_pad_all(pump_status_info->mask_layer, 0, 0);
        lv_obj_clear_flag(pump_status_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
    }
}
#endif

static const char *pump_status_ui_name_get(ui_manager_context_t *thiz, uint16_t popup_type)
{
    switch (popup_type) {
        case PUMP_POPUP_UI_PUMP_FINISH_TIMEOUT:
            return "Ending";
        case PUMP_POPUP_UI_PUMP_FINISH_MANUAL:
            return "Manual Ending";
        case PUMP_POPUP_UI_PUMP_PAUSE_TIMEOUT:
            return "30 Minutes Pausing";
        case PUMP_POPUP_UI_PUMP_PAUSE_MANUAL:
            return "Manual Pausing";
        default:
           return "Unknown";
    }
}

static void pump_status_ui_display_complete(ui_manager_context_t *thiz, uint16_t popup_type)
{
    if (get_timestamp_ms(pump_status_info->start_time) >= pump_status_info->target_time) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        LOG_UI("pump %s display complete", pump_status_ui_name_get(thiz, popup_type));
        ani->progress_cb(thiz, popup_type, ANI_POPUP_DISPLAY_FINISH);
    }
}

static void pump_status_ui_update(ui_manager_context_t *thiz)
{
    switch (pump_status_info->popup_type) {
        case PUMP_POPUP_UI_PUMP_PAUSE_TIMEOUT:
            pump_status_mask_layer_update(thiz);
            pump_status_text_update(thiz, pump_pause_timeout_infos);
            pump_status_icon_update(thiz, RESOURCE_ID_PUMP_PAUSE_PROMPT_ICON);
            break;
        case PUMP_POPUP_UI_PUMP_PAUSE_MANUAL:
            pump_status_mask_layer_update(thiz);
            pump_status_text_update(thiz, pump_pause_manual_infos);
            pump_status_icon_update(thiz, RESOURCE_ID_PUMP_PAUSE_PROMPT_ICON);
            break;
        case PUMP_POPUP_UI_PUMP_FINISH_TIMEOUT:
            pump_status_mask_layer_update(thiz);
            pump_status_text_update(thiz, pump_finish_timeout_infos);
            pump_status_arc_update(thiz);
            pump_status_ui_display_complete(thiz, pump_status_info->popup_type);
            break;
        case PUMP_POPUP_UI_PUMP_FINISH_MANUAL:
            pump_status_mask_layer_update(thiz);
            pump_status_text_update(thiz, pump_finish_manual_infos);
            pump_status_arc_update(thiz);
            pump_status_ui_display_complete(thiz, pump_status_info->popup_type);
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_pump_status_popup_prepare(ui_manager_context_t *thiz, lv_obj_t *parent, uint16_t popup_type)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (pump_status_info == NULL) {
        pump_status_info = (pump_status_display_info_t *)u_malloz(sizeof(pump_status_display_info_t));
        EXCEPTION_CHECK_VOID(pump_status_info);
    }
    pump_status_root_init(parent);
    pump_status_info->ring_progress = 0;
    uint16_t time = 3000;
    pump_status_info->target_time = time;
    pump_status_info->ring_progress_max = (time / PUMP_STATUS_DISPLAY_STEP_TIME) - 2;
    pump_status_info->force_update = true;
    pump_status_info->start_time = get_timestamp();
    pump_status_info->popup_type = popup_type;
    LOG_UI("pump status popup prepare %s", pump_status_ui_name_get(thiz, popup_type));
}

void ui_pump_status_popup_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_status_info == NULL) {
        return;
    }
    common_ui_img_resource_deinit(thiz, &pump_status_info->status_img_info);
    LVGL_OBJ_DELETE(pump_status_info->root);
    u_free(pump_status_info);
    pump_status_info = NULL;
}

void ui_pump_status_popup_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (pump_status_info == NULL || pump_status_info->root == NULL) {
        return;
    }
    pump_status_ui_update(thiz);
    pump_status_info->force_update = false;
}