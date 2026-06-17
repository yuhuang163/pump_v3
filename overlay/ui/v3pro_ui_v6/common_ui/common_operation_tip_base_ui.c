/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define OPERATION_TIP_TEXT_COLOR                                    0xE6DFDF
#define OPERATION_TIP_TEXT_COUNT                                    2
#define OPERATION_STATUS_TEXT_COLOR                                 0xFFFFFF
#define OPERATION_STATUS_ARC_SIZE                                   (360)    // 圆弧尺寸
#define OPERATION_STATUS_ARC_WIDTH                                  (12)      // 圆弧粗细
#define OPERATION_STATUS_ARC_COLOR                                  (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define OPERATION_STATUS_ARC_START_ANGLE                            (270)    
#define OPERATION_STATUS_ARC_FULL_SWEEP                             (360)    
#define OPERATION_STATUS_DISPLAY_STEP_TIME                          (100)
#define OPERATION_PAUSE_ICON_Y                                      (118)

typedef struct {    
    lv_obj_t * root;
    lv_obj_t * icon;
    lv_obj_t * text[OPERATION_TIP_TEXT_COUNT];
    lv_obj_t * mask_layer;            /// 遮罩层对象
    lv_obj_t * arc;                    /// 圆弧进度条对象
    lv_obj_t * number;
    res_img_info_t img_info;          /// 图片信息
    uint32_t start_time;              /// 开始时间
    uint16_t ring_progress;            /// 圆弧进度
    uint16_t ring_progress_max;       /// 圆弧进度最大值
    uint16_t target_time;             /// 倒计时目标时间 单位：秒
    uint8_t display_number;           /// 显示倒计时数字
    uint8_t force_update;
    uint8_t number_max;
    uint8_t rsv[13];
} common_op_tip_disp_info_t;

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint16_t text_opa; /// 文本不透明度
    uint32_t color;
} operation_text_info_t;

typedef struct {
    uint16_t text_y;
    uint16_t img_id;
} operation_icon_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static common_op_tip_disp_info_t *operation_info;

const static operation_text_info_t pump_pause_manual_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Pumping paused", 196, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_STATUS_TEXT_COLOR},
    { "Auto shutoff after 15 minutes of inactivity", 224, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_STATUS_TEXT_COLOR},
};

const static operation_text_info_t pump_pause_timeout_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Pumping over 30 min", 196, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_STATUS_TEXT_COLOR},
    { "Take a short break", 224, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_STATUS_TEXT_COLOR},
};

const static operation_text_info_t pump_finish_manual_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Ending", 180, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_STATUS_TEXT_COLOR},
    { "", 226, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_STATUS_TEXT_COLOR},
};

const static operation_text_info_t pump_finish_timeout_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Pumping timeout", 159, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_STATUS_TEXT_COLOR},
    { "Ending automatically", 190, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_STATUS_TEXT_COLOR},
};

const static operation_text_info_t pump_finish_charge_error_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Incompatible adapter", 159, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_STATUS_TEXT_COLOR},
    { "Ending automatically", 190, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_STATUS_TEXT_COLOR},
};

const static operation_text_info_t pump_finish_low_power_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Low power", 159, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_STATUS_TEXT_COLOR},
    { "Ending automatically", 190, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_STATUS_TEXT_COLOR},
};

const static operation_text_info_t operation_success_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Success", 212, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_TIP_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_TIP_TEXT_COLOR},
};

const static operation_text_info_t operation_success_preset_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Success", 212, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_TIP_TEXT_COLOR},
    { "Take effect durning pumping", 232, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_STATUS_TEXT_COLOR},
};

const static operation_text_info_t operation_failed_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Invalid operation", 212, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_TIP_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_TIP_TEXT_COLOR},
};

const static operation_text_info_t level_adjust_waring_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Adjustment range too large", 212, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_TIP_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_TIP_TEXT_COLOR},
};

const static operation_text_info_t pumping_only_infos[OPERATION_TIP_TEXT_COUNT] = {
    { "Adjust only during pumping", 212, &lv_font_oppo_sans_20_medium, LV_OPA_100, OPERATION_TIP_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, OPERATION_TIP_TEXT_COLOR},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void common_operation_tip_root_init(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(parent);
    if (operation_info->root == NULL || !lv_obj_is_valid(operation_info->root)) {
        operation_info->root = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(operation_info->root);
        lv_obj_set_size(operation_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%
        lv_obj_set_style_bg_color(operation_info->root, lv_color_black(), 0);     /// 黑色背景
        lv_obj_set_style_bg_opa(operation_info->root, LV_OPA_TRANSP, 0);          /// 背景填充透明（不画背景色，避免遮挡底图）
        lv_obj_set_style_border_width(operation_info->root, 0, 0);                /// 无边框
        lv_obj_set_style_pad_all(operation_info->root, 0, 0);                     /// 无内边距
        lv_obj_align(operation_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(operation_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 不拦截点击事件
        lv_obj_clear_flag(operation_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
    }
}

static void common_operation_tip_text_update(ui_manager_context_t *thiz, const operation_text_info_t *text_info)
{
    EXCEPTION_CHECK_VOID(text_info);
    
    if (operation_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < OPERATION_TIP_TEXT_COUNT; i++) {
        if (operation_info->text[i] == NULL || !lv_obj_is_valid(operation_info->text[i])) {
            operation_info->text[i] = lv_label_create(operation_info->root);
            EXCEPTION_CHECK_VOID(operation_info->text[i]);
        }
        lv_label_set_text(operation_info->text[i], text_info[i].name);
        lv_obj_set_style_text_color(operation_info->text[i], lv_color_hex(text_info[i].color), 0);
        lv_obj_set_style_text_font(operation_info->text[i], text_info[i].font, 0);
        lv_obj_set_style_text_opa(operation_info->text[i], text_info[i].text_opa, 0);
        lv_obj_align(operation_info->text[i], LV_ALIGN_TOP_MID, 0, text_info[i].text_y);
    }
}

static void common_operation_tip_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset)
{
    if (operation_info->force_update == false) {
        return;
    }

    if (operation_info->icon == NULL || !lv_obj_is_valid(operation_info->icon)) {
        operation_info->icon = lv_image_create(operation_info->root);
        EXCEPTION_CHECK_VOID(operation_info->icon);
    }

    common_ui_theme_img_resource_init(thiz, img_id, RES_THEME_TYPE_BASE,
        &operation_info->img_info);
    EXCEPTION_CHECK_VOID(operation_info->img_info.data);
    lv_fs_path_ex_t invalid_operation_icon_img_path;
    lv_fs_make_path_from_buffer(&invalid_operation_icon_img_path, LV_FS_MEMFS_LETTER,
        operation_info->img_info.data, operation_info->img_info.size, "baj");
    lv_image_set_src(operation_info->icon, invalid_operation_icon_img_path.path);
    lv_obj_align(operation_info->icon, LV_ALIGN_TOP_MID, 0, y_offset);
}

static void common_operation_arc_update(ui_manager_context_t *thiz)
{
    uint16_t ring_progress = get_timestamp_ms(operation_info->start_time) / OPERATION_STATUS_DISPLAY_STEP_TIME;
    if (operation_info->force_update == false && operation_info->ring_progress == ring_progress) {
        return;
    }

    operation_info->ring_progress = ring_progress;
    if (operation_info->arc == NULL || !lv_obj_is_valid(operation_info->arc)) {
        operation_info->arc = lv_arc_create(operation_info->root);
        EXCEPTION_CHECK_VOID(operation_info->arc);
        lv_obj_set_size(operation_info->arc, OPERATION_STATUS_ARC_SIZE, OPERATION_STATUS_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(operation_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(operation_info->arc, 0, OPERATION_STATUS_ARC_SIZE);  /// 背景弧为完整闭环
        lv_arc_set_angles(operation_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(operation_info->arc, OPERATION_STATUS_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(operation_info->arc, lv_color_hex(OPERATION_STATUS_ARC_START_ANGLE), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(operation_info->arc, OPERATION_STATUS_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(operation_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(operation_info->arc, lv_color_hex(OPERATION_STATUS_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(operation_info->arc, OPERATION_STATUS_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(operation_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(operation_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (ring_progress >= operation_info->ring_progress_max) {
        lv_obj_set_style_arc_opa(operation_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(operation_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(operation_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(operation_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(operation_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (OPERATION_STATUS_ARC_FULL_SWEEP * ring_progress) / operation_info->ring_progress_max;
    }
    lv_arc_set_angles(operation_info->arc, 0, end_angle);
}

static void common_operation_num_update(ui_manager_context_t *thiz)
{
    if (operation_info->number == NULL || !lv_obj_is_valid(operation_info->number)) {
        operation_info->number = lv_label_create(operation_info->root);
        EXCEPTION_CHECK_VOID(operation_info->number);
    }
    char num_buf[8] = {0};
    uint8_t num = get_timestamp_ms(operation_info->start_time) / 1000;
    uint8_t show_num = (num > operation_info->number_max ? 0 : operation_info->number_max - num);
    if (operation_info->force_update == false &&
        operation_info->display_number == show_num) {
        return;
    }
    snprintf(num_buf, sizeof(num_buf), "%d", show_num == 0 ? 1 : show_num);
    lv_label_set_text(operation_info->number, num_buf);
    lv_obj_set_style_text_color(operation_info->number, lv_color_white(), 0);
    lv_obj_set_style_text_font(operation_info->number, &lv_font_oppo_sans_96_bold, 0);
    lv_obj_set_style_text_align(operation_info->number, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(operation_info->number, LV_ALIGN_CENTER, 0, 0);
    operation_info->display_number = show_num;
}

static void common_operation_tip_mask_layer_update(ui_manager_context_t *thiz)
{
    if (operation_info->force_update == false) {
        return;
    }
    if (operation_info->mask_layer == NULL || !lv_obj_is_valid(operation_info->mask_layer)) {
        operation_info->mask_layer = lv_obj_create(operation_info->root);
        EXCEPTION_CHECK_VOID(operation_info->mask_layer);
    }
    lv_obj_set_size(operation_info->mask_layer, LV_PCT(100), LV_PCT(100));
    lv_obj_center(operation_info->mask_layer);
    lv_obj_set_style_bg_color(operation_info->mask_layer, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(operation_info->mask_layer, 190, 0); /// 黑色背景透明度190
    lv_obj_set_style_border_width(operation_info->mask_layer, 0, 0);
    lv_obj_set_style_pad_all(operation_info->mask_layer, 0, 0);
    lv_obj_clear_flag(operation_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
}

static const char *common_operation_tip_name_get(ui_manager_context_t *thiz, uint16_t popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
            return "pump Ending";
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
            return "pump Manual Ending";
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
            return "Incompatible adapter";
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
            return "Low power";
        case ID_POPUP_UI_PUMP_PAUSE_TIMEOUT:
            return "pump 30 Minutes Pausing";
        case ID_POPUP_UI_PUMP_PAUSE_MANUAL:
            return "pump Manual Pausing";
        case ID_POPUP_UI_OP_SUCCESS:
            return "operation success";
        case ID_POPUP_UI_OP_SUCCESS_PRESET:
            return "operation success preset";
        case ID_POPUP_UI_OP_FAILED:
            return "operation failed";
        case ID_POPUP_UI_LEVEL_ADJUST_WARNING:
            return "pump level adjust error";
        case ID_POPUP_UI_ONLY_AVAILABLE_IN_PUMPING:
            return "no pumping";
        default:
           return "Unknown";
    }
}

static void common_operation_tip_display_complete(ui_manager_context_t *thiz, uint16_t popup_type)
{
    uint32_t time_diff = get_timestamp_ms(operation_info->start_time) + OPERATION_STATUS_DISPLAY_STEP_TIME;
    if (time_diff >= operation_info->target_time) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        LOG_UI("%s display complete", common_operation_tip_name_get(thiz, popup_type));
        ani->progress_cb(thiz, popup_type, ANI_POPUP_DISPLAY_FINISH);
    }
}

static void common_operation_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_PUMP_PAUSE_TIMEOUT:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, pump_pause_timeout_infos);
            common_operation_tip_icon_update(thiz, RESOURCE_ID_PUMP_PAUSE_ICON, 118);
            break;
        case ID_POPUP_UI_PUMP_PAUSE_MANUAL:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, pump_pause_manual_infos);
            common_operation_tip_icon_update(thiz, RESOURCE_ID_PUMP_PAUSE_ICON, 118);
            break;
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, pump_finish_timeout_infos);
            common_operation_arc_update(thiz);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, pump_finish_manual_infos);
            common_operation_arc_update(thiz);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, pump_finish_charge_error_infos);
            common_operation_arc_update(thiz);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, pump_finish_low_power_infos);
            common_operation_arc_update(thiz);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_OP_SUCCESS:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, operation_success_infos);
            common_operation_tip_icon_update(thiz, RESOURCE_ID_ALLOW_ICON, 130);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_OP_SUCCESS_PRESET:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, operation_success_preset_infos);
            common_operation_tip_icon_update(thiz, RESOURCE_ID_ALLOW_ICON, 130);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_OP_FAILED:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, operation_failed_infos);
            common_operation_tip_icon_update(thiz, RESOURCE_ID_BAN_ICON, 130);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_LEVEL_ADJUST_WARNING:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, level_adjust_waring_infos);
            common_operation_tip_icon_update(thiz, RESOURCE_ID_WARNING_ICON, 130);
            break;
        case ID_POPUP_UI_ONLY_AVAILABLE_IN_PUMPING:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_tip_text_update(thiz, pumping_only_infos);
            common_operation_tip_icon_update(thiz, RESOURCE_ID_WARNING_ICON, 130);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_AIRPLANE_MODE_PROMPT:
            common_operation_tip_mask_layer_update(thiz);
            common_operation_arc_update(thiz);
            common_operation_num_update(thiz);
            common_operation_tip_display_complete(thiz, popup_type);
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_common_operation_tip_base_ui_prepare(ui_manager_context_t *thiz, lv_obj_t *parent, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (operation_info == NULL) {
        operation_info = (common_op_tip_disp_info_t *)u_malloz(sizeof(common_op_tip_disp_info_t));
        EXCEPTION_CHECK_VOID(operation_info);
    }
    common_operation_tip_root_init(thiz, parent);
    operation_info->ring_progress = 0;
    uint16_t time = 3000;
    if (type == ID_POPUP_UI_PUMP_FINISH_MANUAL || type == ID_POPUP_UI_OP_SUCCESS || 
        type == ID_POPUP_UI_OP_FAILED || type == ID_POPUP_UI_ONLY_AVAILABLE_IN_PUMPING ||
        type == ID_POPUP_UI_OP_SUCCESS_PRESET) {
        time = 1000;
    } else if (type == ID_POPUP_UI_AIRPLANE_MODE_PROMPT) {
        time = 5000;
    }
    operation_info->target_time = time;
    operation_info->number_max = time / 1000;
    operation_info->ring_progress_max = (time / OPERATION_STATUS_DISPLAY_STEP_TIME) - 2;
    operation_info->force_update = true;
    operation_info->start_time = get_timestamp();
    LOG_UI("pump status popup prepare %s", common_operation_tip_name_get(thiz, type));
}

void ui_common_operation_tip_base_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (operation_info == NULL) {
        return;
    }
    common_ui_img_resource_deinit(thiz, &operation_info->img_info);
    LVGL_OBJ_DELETE(operation_info->root);
    u_free(operation_info);
    operation_info = NULL;
}

void ui_common_operation_tip_base_ui_process(ui_manager_context_t *thiz, popup_ui_id_e type)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (operation_info == NULL || operation_info->root == NULL ||
        !lv_obj_is_valid(operation_info->root)) {
        return;
    }
    common_operation_ui_update(thiz, type);
    operation_info->force_update = false;
}

void ui_common_operation_tip_base_ui_update(ui_manager_context_t *thiz,
    popup_ui_id_e type, lv_obj_t *parent, uint8_t *ui_index)
{
    switch (type) {
        default:
        case ID_POPUP_UI_NONE:
            ui_common_operation_tip_base_ui_remove(thiz);
            *ui_index = ID_POPUP_UI_NONE;
            break;
        case ID_POPUP_UI_OP_SUCCESS:
        case ID_POPUP_UI_OP_SUCCESS_PRESET:
        case ID_POPUP_UI_OP_FAILED:
        case ID_POPUP_UI_LEVEL_ADJUST_WARNING:
        case ID_POPUP_UI_ONLY_AVAILABLE_IN_PUMPING:
        case ID_POPUP_UI_PUMP_FINISH_TIMEOUT:
        case ID_POPUP_UI_PUMP_FINISH_MANUAL:
        case ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR:
        case ID_POPUP_UI_PUMP_FINISH_LOW_POWER:
        case ID_POPUP_UI_PUMP_PAUSE_MANUAL:
        case ID_POPUP_UI_PUMP_PAUSE_TIMEOUT:
        case ID_POPUP_UI_AIRPLANE_MODE_PROMPT:
            if (*ui_index != type) {
                ui_common_operation_tip_base_ui_remove(thiz);
                ui_common_operation_tip_base_ui_prepare(thiz, parent, type);
                *ui_index = type;
            } else {
                ui_common_operation_tip_base_ui_process(thiz, type);
            }
            break;
    }
}

