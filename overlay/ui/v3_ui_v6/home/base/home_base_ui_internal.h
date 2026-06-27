#ifndef __HOME_BASE_UI_INTERNAL_H__
#define __HOME_BASE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef enum {
    HOME_BASE_LEVEL_LEFT = 0,
    HOME_BASE_LEVEL_RIGHT = 1,
    HOME_BASE_LEVEL_MAX,
} home_base_level_id_e;

typedef struct {
    lv_obj_t *arc;
    lv_obj_t *indicator; /// 指示符
    lv_obj_t *progress; /// 进度文字
    lv_obj_t *progress_bg; /// 进度背景 
    uint8_t value; /// 级别值
    uint8_t new_value; /// 新级别值
    uint8_t adjust_active; /// 0: 默认样式, 1: 调节样式.
} home_base_level_t;

typedef struct {
    home_base_level_t left;
    home_base_level_t right;
    lv_obj_t *separator; /// 分隔符
    uint8_t left_blink_state; /// 左力度闪烁: 0: 不闪烁, 1: 闪烁.
    uint8_t right_blink_state; /// 右力度闪烁: 0: 不闪烁, 1: 闪烁.
} home_base_level_info_t;

typedef struct {
    lv_obj_t *label;
    lv_obj_t *icon;
    lv_obj_t *text;
    lv_obj_t *time; /// 模式时长
    uint8_t type; /// 标签类型
    uint8_t force_update; /// 强制更新
    uint8_t text_id;        /// 已显示文本索引
    uint16_t label_img_id; /// 已解码标签背景资源 ID，0 表示隐藏
    uint16_t icon_img_id; /// 已解码图标资源 ID，0 表示隐藏
    uint16_t label_bg_x; /// 标签背景 X 偏移
    uint16_t label_bg_y; /// 标签背景 Y 偏移
    char time_text[16]; /// 已显示时长文本
} home_base_label_info_t;

typedef struct {
    lv_obj_t *title;
    lv_obj_t *icon;
    lv_obj_t *wave; /// 模式波形
    uint8_t index; /// 模式索引
    uint8_t new_index; /// 新模式索引
    uint8_t mode_blink_state; /// 模式标题闪烁: 0: 不闪烁, 1: 闪烁.
    uint8_t icon_blink_state; /// 韵律图标闪烁: 0: 不闪烁, 1: 闪烁.
    uint32_t custom_mode_id; /// 个性化模式ID
} home_base_mode_info_t;

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    home_base_level_info_t level;
    home_base_mode_info_t mode;
    home_base_label_info_t label;
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t pump_status;
    uint8_t popup_ui_index;
    uint8_t force_update;
    uint8_t sub_ui_status;
    uint32_t state_blink_time; /// 状态闪烁时间.
} home_base_ui_info_t;

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_pump_mode_ui_remove(home_base_ui_info_t *info);
void home_pump_mode_ui_update(home_base_ui_info_t *info);

void home_pump_level_ui_remove(home_base_ui_info_t *info);
void home_pump_level_ui_update(home_base_ui_info_t *info);

void home_pump_label_ui_remove(home_base_ui_info_t *info);
void home_pump_label_ui_update(home_base_ui_info_t *info);

bool home_base_blink_visible_get(const home_base_ui_info_t *info);
void home_base_blink_state_update(home_base_ui_info_t *info,
    uint8_t pump_mode_display_type, int pump_mode, uint8_t pump_level_display_type);

void home_base_ui_prepare(ui_manager_context_t *thiz);
void home_base_ui_remove(ui_manager_context_t *thiz);
int home_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __HOME_BASE_UI_INTERNAL_H__

