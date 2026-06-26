/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_popup_window.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_popup.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define POPUP_TIP_TEXT_COLOR                                    0xE6DFDF
#define POPUP_TIP_TEXT_COUNT                                    2
#define POPUP_STATUS_TEXT_COLOR                                 0xFFFFFF
#define POPUP_STATUS_ARC_SIZE                                   (360)    // 圆弧尺寸
#define POPUP_STATUS_ARC_WIDTH                                  (12)      // 圆弧粗细
#define POPUP_STATUS_ARC_COLOR                                  (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define POPUP_STATUS_ARC_START_ANGLE                            (270)    
#define POPUP_STATUS_ARC_FULL_SWEEP                             (360)    
#define POPUP_STATUS_DISPLAY_STEP_TIME                          (100)
#define POPUP_PAUSE_ICON_Y                                      (118)

typedef struct {    
    lv_obj_t * root;
    lv_obj_t * icon;
    lv_obj_t * text[POPUP_TIP_TEXT_COUNT];
    lv_obj_t * mask_layer;            /// 遮罩层对象
    lv_obj_t * arc;                    /// 圆弧进度条对象
    lv_obj_t * number;
    res_img_info_t img_info;          /// 图片信息
    res_theme_type_e res_theme_type;
    uint32_t start_time;              /// 开始时间
    uint16_t ring_progress;            /// 圆弧进度
    uint16_t ring_progress_max;       /// 圆弧进度最大值
    uint16_t target_time;             /// 倒计时目标时间 单位：秒
    uint8_t display_number;           /// 显示倒计时数字
    uint8_t force_update;
    uint8_t number_max;         
    uint8_t rsv[13];
} popup_tip_disp_info_t;

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint16_t text_opa; /// 文本不透明度
    uint32_t color;
} popup_text_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static popup_tip_disp_info_t *popup_info;

const static popup_text_info_t low_power_infos[POPUP_TIP_TEXT_COUNT] = {
    { "Powering off in", 111, &lv_font_oppo_sans_16_medium, LV_OPA_100, 0xC74B57},
    { "", 0, &lv_font_oppo_sans_14_medium, LV_OPA_70, 0xC74B57},
};

const static popup_text_info_t will_power_off_infos[POPUP_TIP_TEXT_COUNT] = {
    { "Shut down soon", 111, &lv_font_oppo_sans_16_medium, LV_OPA_100, 0xC74B57},
    { "", 0, &lv_font_oppo_sans_14_medium, LV_OPA_70, 0xC74B57},
};

const static popup_text_info_t ble_connect_infos[POPUP_TIP_TEXT_COUNT] = {
    { "Device connected", 210, &lv_font_oppo_sans_16_medium, LV_OPA_100, POPUP_STATUS_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, POPUP_STATUS_TEXT_COLOR},
};

const static popup_text_info_t ble_disconnect_infos[POPUP_TIP_TEXT_COUNT] = {
    { "Device disconnected", 210, &lv_font_oppo_sans_16_medium, LV_OPA_100, POPUP_STATUS_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, POPUP_STATUS_TEXT_COLOR},
};

const static popup_text_info_t ble_connecting_infos[POPUP_TIP_TEXT_COUNT] = {
    { "Connecting", 210, &lv_font_oppo_sans_16_medium, LV_OPA_100, POPUP_STATUS_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, POPUP_STATUS_TEXT_COLOR},
};

const static popup_text_info_t device_bind_infos[POPUP_TIP_TEXT_COUNT] = {
    { "Paired successfully", 210, &lv_font_oppo_sans_16_medium, LV_OPA_100, POPUP_STATUS_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, POPUP_STATUS_TEXT_COLOR},
};

const static popup_text_info_t device_unbind_infos[POPUP_TIP_TEXT_COUNT] = {
    { "", 0, &lv_font_oppo_sans_20_medium, LV_OPA_100, POPUP_STATUS_TEXT_COLOR},
    { "", 0, &lv_font_oppo_sans_16_medium, LV_OPA_70, POPUP_STATUS_TEXT_COLOR},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void popup_window_ui_root_init(ui_manager_context_t *thiz)
{
    if (popup_info->root == NULL || !lv_obj_is_valid(popup_info->root)) {
        popup_info->root = lv_obj_create(lv_layer_top());
        EXCEPTION_CHECK_VOID(popup_info->root);
        lv_obj_set_size(popup_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%
        lv_obj_set_style_bg_color(popup_info->root, lv_color_black(), 0);     /// 黑色背景
        lv_obj_set_style_bg_opa(popup_info->root, LV_OPA_TRANSP, 0);          /// 背景填充透明（不画背景色，避免遮挡底图）
        lv_obj_set_style_border_width(popup_info->root, 0, 0);                /// 无边框
        lv_obj_set_style_pad_all(popup_info->root, 0, 0);                     /// 无内边距
        lv_obj_align(popup_info->root, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(popup_info->root, LV_OBJ_FLAG_CLICKABLE);           /// 不拦截点击事件
        lv_obj_clear_flag(popup_info->root, LV_OBJ_FLAG_SCROLLABLE);          /// 禁止滚动
    }
}

static void popup_window_ui_text_update(ui_manager_context_t *thiz, const popup_text_info_t *text_info)
{
    EXCEPTION_CHECK_VOID(text_info);
    
    if (popup_info->force_update == false) {
        return;
    }

    for (uint8_t i = 0; i < POPUP_TIP_TEXT_COUNT; i++) {
        if (popup_info->text[i] == NULL || !lv_obj_is_valid(popup_info->text[i])) {
            popup_info->text[i] = lv_label_create(popup_info->root);
            EXCEPTION_CHECK_VOID(popup_info->text[i]);
        }
        lv_label_set_text(popup_info->text[i], text_info[i].name);
        lv_obj_set_style_text_color(popup_info->text[i], lv_color_hex(text_info[i].color), 0);
        lv_obj_set_style_text_font(popup_info->text[i], text_info[i].font, 0);
        lv_obj_set_style_text_opa(popup_info->text[i], text_info[i].text_opa, 0);
        lv_obj_align(popup_info->text[i], LV_ALIGN_TOP_MID, 0, text_info[i].text_y);
    }
}

static void popup_window_ui_icon_update(ui_manager_context_t *thiz, uint16_t img_id, uint16_t y_offset)
{
    if (popup_info->force_update == false) {
        return;
    }

    if (popup_info->icon == NULL || !lv_obj_is_valid(popup_info->icon)) {
        popup_info->icon = lv_image_create(popup_info->root);
        EXCEPTION_CHECK_VOID(popup_info->icon);
    }
    ui_theme_img_resource_init(thiz, img_id, popup_info->res_theme_type,
        &popup_info->img_info);
    EXCEPTION_CHECK_VOID(popup_info->img_info.data);
    lv_fs_path_ex_t invalid_popup_icon_img_path;
    lv_fs_make_path_from_buffer(&invalid_popup_icon_img_path, LV_FS_MEMFS_LETTER,
        popup_info->img_info.data, popup_info->img_info.size, "baj");
    lv_image_set_src(popup_info->icon, invalid_popup_icon_img_path.path);
    lv_obj_align(popup_info->icon, LV_ALIGN_TOP_MID, 0, y_offset);
}

static void popup_window_ui_arc_update(ui_manager_context_t *thiz)
{
    uint16_t ring_progress = get_timestamp_ms(popup_info->start_time) / POPUP_STATUS_DISPLAY_STEP_TIME;
    if (popup_info->force_update == false && popup_info->ring_progress == ring_progress) {
        return;
    }

    popup_info->ring_progress = ring_progress;
    if (popup_info->arc == NULL || !lv_obj_is_valid(popup_info->arc)) {
        popup_info->arc = lv_arc_create(popup_info->root);
        EXCEPTION_CHECK_VOID(popup_info->arc);
        lv_obj_set_size(popup_info->arc, POPUP_STATUS_ARC_SIZE, POPUP_STATUS_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(popup_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(popup_info->arc, 0, POPUP_STATUS_ARC_SIZE);  /// 背景弧为完整闭环
        lv_arc_set_angles(popup_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(popup_info->arc, POPUP_STATUS_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(popup_info->arc, lv_color_hex(POPUP_STATUS_ARC_START_ANGLE), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(popup_info->arc, POPUP_STATUS_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(popup_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(popup_info->arc, lv_color_hex(POPUP_STATUS_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(popup_info->arc, POPUP_STATUS_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(popup_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(popup_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    if (ring_progress >= popup_info->ring_progress_max) {
        lv_obj_set_style_arc_opa(popup_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(popup_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(popup_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(popup_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(popup_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (POPUP_STATUS_ARC_FULL_SWEEP * ring_progress) / popup_info->ring_progress_max;
    }
    lv_arc_set_angles(popup_info->arc, 0, end_angle);
}

static void popup_window_ui_num_update(ui_manager_context_t *thiz)
{
    if (popup_info->number == NULL || !lv_obj_is_valid(popup_info->number)) {
        popup_info->number = lv_label_create(popup_info->root);
        EXCEPTION_CHECK_VOID(popup_info->number);
    }
    char num_buf[8] = {0};
    uint8_t num = get_timestamp_ms(popup_info->start_time) / 1000;
    uint8_t show_num = (num > popup_info->number_max ? 0 : popup_info->number_max - num);
    if (popup_info->force_update == false &&
        popup_info->display_number == show_num) {
        return;
    }
    snprintf(num_buf, sizeof(num_buf), "%d", show_num == 0 ? 1 : show_num);
    lv_label_set_text(popup_info->number, num_buf);
    lv_obj_set_style_text_color(popup_info->number, lv_color_white(), 0);
    lv_obj_set_style_text_font(popup_info->number, &lv_font_oppo_sans_96_bold, 0);
    lv_obj_set_style_text_align(popup_info->number, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(popup_info->number, LV_ALIGN_CENTER, 0, 0);
    popup_info->display_number = show_num;
}

static void popup_window_ui_mask_layer_update(ui_manager_context_t *thiz)
{
    if (popup_info->force_update == false) {
        return;
    }

    if (popup_info->mask_layer == NULL || !lv_obj_is_valid(popup_info->mask_layer)) {
        popup_info->mask_layer = lv_obj_create(popup_info->root);
        EXCEPTION_CHECK_VOID(popup_info->mask_layer);
    }
    lv_obj_set_size(popup_info->mask_layer, LV_PCT(100), LV_PCT(100));
    lv_obj_center(popup_info->mask_layer);
    lv_obj_set_style_bg_color(popup_info->mask_layer, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(popup_info->mask_layer, 190, 0); /// 黑色背景透明度190
    lv_obj_set_style_border_width(popup_info->mask_layer, 0, 0);
    lv_obj_set_style_pad_all(popup_info->mask_layer, 0, 0);
    lv_obj_clear_flag(popup_info->mask_layer, LV_OBJ_FLAG_CLICKABLE);
}

static const char *popup_window_ui_name_get(ui_manager_context_t *thiz, uint16_t popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_BT_CONNECT:
            return "ble connect";
        case ID_POPUP_UI_BT_DISCONNECT:
            return "ble disconnect";
        case ID_POPUP_UI_BT_CONNECTING:
            return "ble connecting";
        case ID_POPUP_UI_DEVICE_BIND:
            return "device bind";
        case ID_POPUP_UI_DEVICE_UNBIND:
            return "device unbind";
        case ID_POPUP_UI_COUNT_DOWN:
            return "count down";
        case ID_POPUP_UI_LOW_POWER:
            return "low power";
        case ID_POPUP_UI_WILL_POWER_OFF:
            return "will power off";
        default:
           return "Unknown";
    }
}

static void popup_window_ui_display_complete(ui_manager_context_t *thiz, uint16_t popup_type)
{
    uint32_t time_diff = get_timestamp_ms(popup_info->start_time) + POPUP_STATUS_DISPLAY_STEP_TIME;
    if (time_diff >= popup_info->target_time) {
        ui_animation_play_t *ani = &thiz->priv.overlay_play_info;
        EXCEPTION_CHECK_VOID(ani);
        LOG_UI("%s display complete", popup_window_ui_name_get(thiz, popup_type));
        ani->progress_cb(thiz, popup_type, ANI_POPUP_DISPLAY_FINISH);
    }
}

static void popup_window_ui_update(ui_manager_context_t *thiz, popup_ui_id_e popup_type)
{
    switch (popup_type) {
        case ID_POPUP_UI_BT_CONNECT:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_text_update(thiz, ble_connect_infos);
            popup_window_ui_icon_update(thiz, BASE_RES_ID_BLE_CONNECT_POPUP_ICON, 130);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_BT_DISCONNECT:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_text_update(thiz, ble_disconnect_infos);
            popup_window_ui_icon_update(thiz, BASE_RES_ID_BLE_DISCONNECT_POPUP_ICON, 130);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_BT_CONNECTING:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_text_update(thiz, ble_connecting_infos);
            popup_window_ui_icon_update(thiz, BASE_RES_ID_BLE_DISCONNECT_POPUP_ICON, 130);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_DEVICE_BIND:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_text_update(thiz, device_bind_infos);
            popup_window_ui_icon_update(thiz, BASE_RES_ID_BIND_ICON, 130);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_DEVICE_UNBIND:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_text_update(thiz, device_unbind_infos);
            popup_window_ui_icon_update(thiz, BASE_RES_ID_UNBIND_ICON, 130);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_COUNT_DOWN:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_arc_update(thiz);
            popup_window_ui_num_update(thiz);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_LOW_POWER:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_text_update(thiz, low_power_infos);
            popup_window_ui_arc_update(thiz);
            popup_window_ui_num_update(thiz);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        case ID_POPUP_UI_WILL_POWER_OFF:
            popup_window_ui_mask_layer_update(thiz);
            popup_window_ui_text_update(thiz, will_power_off_infos);
            popup_window_ui_arc_update(thiz);
            popup_window_ui_num_update(thiz);
            popup_window_ui_display_complete(thiz, popup_type);
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void popup_window_ui_prepare(ui_manager_context_t *thiz, popup_ui_id_e type, uint16_t time)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (popup_info == NULL) {
        popup_info = (popup_tip_disp_info_t *)u_malloz(sizeof(popup_tip_disp_info_t));
        EXCEPTION_CHECK_VOID(popup_info);
    }
    popup_window_ui_root_init(thiz);
    popup_info->ring_progress = 0;
    popup_info->target_time = time;
    popup_info->number_max = time / 1000;
    popup_info->ring_progress_max = (time / POPUP_STATUS_DISPLAY_STEP_TIME) - 2;
    popup_info->force_update = true;
    popup_info->start_time = get_timestamp();
    popup_info->res_theme_type = RES_THEME_TYPE_BASE;
    if (thiz && thiz->priv.res) {
        popup_info->res_theme_type  = thiz->priv.res->get_current_theme_type();
    }
    LOG_UI("popup: %s, time:%d", popup_window_ui_name_get(thiz, type), time);
}

void popup_window_ui_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (popup_info == NULL) {
        return;
    }
    ui_img_resource_deinit(thiz, &popup_info->img_info);
    LVGL_OBJ_DELETE(popup_info->root);
    u_free(popup_info);
    popup_info = NULL;
}

int popup_window_ui_process(ui_manager_context_t *thiz, popup_ui_id_e type)
{
    EXCEPTION_CHECK_INT(thiz && popup_info && popup_info->root);
    // ui_proc_popup_t *proc = (ui_proc_popup_t *) thiz->priv.overlay_play_info.user_data;
    popup_window_ui_update(thiz, type);
    popup_info->force_update = false;
    return 0;
}