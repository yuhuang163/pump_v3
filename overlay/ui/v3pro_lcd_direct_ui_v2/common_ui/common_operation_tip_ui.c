/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep
#include <stdint.h>

/*
* DEFINES
*****************************************************************************************
*/
#define OPERATION_TIP_TEXT_COLOR                                    0xE6DFDF

typedef struct {    
    lv_obj_t * root;
    lv_obj_t * icon;
    lv_obj_t * text;           /// 文本对象
    lv_obj_t * mask_layer;            /// 遮罩层对象
    res_img_info_t img_info;          /// 图片信息
    uint8_t force_update;
    uint8_t rsv[15];
} common_op_tip_disp_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static common_op_tip_disp_info_t *operation_info;

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

static void common_operation_tip_text_update(ui_manager_context_t *thiz, common_operation_type_e type)
{
    if (operation_info->text == NULL || !lv_obj_is_valid(operation_info->text)) {
        operation_info->text = lv_label_create(operation_info->root);
        EXCEPTION_CHECK_VOID(operation_info->text);
    }
    if (type == COMMON_OPERATION_SUCCESS) {
        lv_label_set_text(operation_info->text, "success");
    } else if (type == COMMON_OPERATION_FAILED) {
        lv_label_set_text(operation_info->text, "Invalid operation");
    } else if (type == COMMON_OPERATION_LEVEL_ADJUST_TOO_LARGE) {
        lv_label_set_text(operation_info->text, "Adjustment range too large");
    }
    lv_obj_set_style_text_color(operation_info->text, lv_color_hex(OPERATION_TIP_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(operation_info->text, &lv_font_oppo_sans_16_medium, 0);
    lv_obj_align(operation_info->text, LV_ALIGN_TOP_MID, 0, 210 + 2);
}

static void common_operation_tip_icon_update(ui_manager_context_t *thiz, common_operation_type_e type)
{
    if (operation_info->icon == NULL || !lv_obj_is_valid(operation_info->icon)) {
        operation_info->icon = lv_image_create(operation_info->root);
        EXCEPTION_CHECK_VOID(operation_info->icon);
    }
    if (type == COMMON_OPERATION_SUCCESS) {
        common_ui_img_resource_init(thiz, RESOURCE_ID_ALLOW_ICON, &operation_info->img_info);
    } else if (type == COMMON_OPERATION_FAILED) {
        common_ui_img_resource_init(thiz, RESOURCE_ID_BAN_ICON, &operation_info->img_info);
    } else if (type == COMMON_OPERATION_LEVEL_ADJUST_TOO_LARGE) {
        common_ui_img_resource_init(thiz, RESOURCE_ID_WARNING_ICON, &operation_info->img_info);
    }
    EXCEPTION_CHECK_VOID(operation_info->img_info.data);
    lv_fs_path_ex_t invalid_operation_icon_img_path;
    lv_fs_make_path_from_buffer(&invalid_operation_icon_img_path, LV_FS_MEMFS_LETTER,
        operation_info->img_info.data, operation_info->img_info.size, "baj");
    lv_image_set_src(operation_info->icon, invalid_operation_icon_img_path.path);
    lv_obj_align(operation_info->icon, LV_ALIGN_TOP_MID, 0, 130);
}

static void common_operation_tip_mask_layer_update(ui_manager_context_t *thiz)
{
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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_common_operation_tip_prepare(ui_manager_context_t *thiz, lv_obj_t *parent)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(parent);
    if (operation_info == NULL) {
        operation_info = (common_op_tip_disp_info_t *)u_malloz(sizeof(common_op_tip_disp_info_t));
        EXCEPTION_CHECK_VOID(operation_info);
    }
    operation_info->force_update = true;
    common_operation_tip_root_init(thiz, parent);
}

void ui_common_operation_tip_remove(ui_manager_context_t *thiz)
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

void ui_common_operation_tip_process(ui_manager_context_t *thiz, common_operation_type_e type, uint32_t start_time)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (operation_info == NULL || operation_info->root == NULL ||
        !lv_obj_is_valid(operation_info->root)) {
        return;
    }
    
    if (operation_info->force_update) {
        common_operation_tip_mask_layer_update(thiz);
        common_operation_tip_text_update(thiz, type);
        common_operation_tip_icon_update(thiz, type);
        operation_info->force_update = false;
    }

    if (get_timestamp_ms(start_time) >= OPERATION_SUCCESS_DISPLAY_TIMEOUT) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_VOID(ani);
        if (type == COMMON_OPERATION_SUCCESS) {
            ani->progress_cb(thiz, PUMP_POPUP_UI_OPERATION_SUCCESS, ANI_POPUP_DISPLAY_FINISH);
        } else if (type == COMMON_OPERATION_FAILED) {
            ani->progress_cb(thiz, PUMP_POPUP_UI_OPERATION_FAILED, ANI_POPUP_DISPLAY_FINISH);
        }
    }
}
