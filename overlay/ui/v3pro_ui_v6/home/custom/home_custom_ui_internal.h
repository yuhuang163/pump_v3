#ifndef __HOME_CUSTOM_UI_INTERNAL_H__
#define __HOME_CUSTOM_UI_INTERNAL_H__

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
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if HOME_MODE_SELECT_TRANSITION_ENABLE

#if HOME_MODE_SELECT_CAPSULE_ENABLE
#define HOME_CUSTOM_MODE_SELECT_CAPSULE_MEMBERS \
    lv_obj_t *title_capsule; \
    lv_obj_t *capsule_title; \
    lv_obj_t *capsule_icon; \
    lv_obj_t *select_dots[3]; \
    uint8_t select_dot_count;
#else
#define HOME_CUSTOM_MODE_SELECT_CAPSULE_MEMBERS
#endif

#if !HOME_MODE_SELECT_CAPSULE_ENABLE
#define HOME_CUSTOM_MODE_SELECT_CAROUSEL_MEMBERS \
    lv_obj_t *select_titles[3]; \
    int16_t select_title_slide_offset; \
    int8_t select_slide_dir;
#else
#define HOME_CUSTOM_MODE_SELECT_CAROUSEL_MEMBERS
#endif

#define HOME_CUSTOM_MODE_SELECT_COMMON_MEMBERS \
    uint8_t display_type; \
    uint8_t select_ui_active; \
    uint8_t select_capsule_style; \
    uint8_t custom_mode_active;

#else

#define HOME_CUSTOM_MODE_SELECT_CAPSULE_MEMBERS
#define HOME_CUSTOM_MODE_SELECT_CAROUSEL_MEMBERS
#define HOME_CUSTOM_MODE_SELECT_COMMON_MEMBERS

#endif

#define HOME_CUSTOM_MODE_SELECT_LABEL_OFFSET_MEMBERS \
    int16_t select_label_y_offset; \
    int16_t select_time_y_offset;

typedef enum {
    HOME_CUSTOM_LEVEL_LEFT = 0,
    HOME_CUSTOM_LEVEL_RIGHT = 1,
    HOME_CUSTOM_LEVEL_MAX,
} home_custom_level_id_e;

typedef struct {
    lv_obj_t *arc;
    lv_obj_t *indicator; /// 指示符
    lv_obj_t *progress; /// 进度文字
    lv_obj_t *progress_bg; /// 进度背景
    uint8_t value; /// 级别值
    uint8_t new_value; /// 新级别值
    uint8_t adjust_active; /// 0: 默认样式, 1: 调节样式.
} home_custom_level_t;

typedef struct {
    home_custom_level_t left;
    home_custom_level_t right;
    lv_obj_t *separator; /// 分隔符
    uint8_t left_blink_state; /// 左力度闪烁: 0: 不闪烁, 1: 闪烁.
    uint8_t right_blink_state; /// 右力度闪烁: 0: 不闪烁, 1: 闪烁.
} home_custom_level_info_t;

typedef struct {
    lv_obj_t *label;
    lv_obj_t *icon;
    lv_obj_t *text;
    lv_obj_t *button[2];
    lv_obj_t *time; /// 模式时长
    uint8_t type; /// 标签类型
    uint8_t force_update; /// 强制更新
    uint8_t text_id;        /// 已显示文本索引
    uint16_t label_img_id; /// 已解码标签背景资源 ID，0 表示隐藏
    uint16_t icon_img_id; /// 已解码图标资源 ID，0 表示隐藏
    uint16_t button_img_id[2]; /// 已解码按钮图标资源 ID，0 表示隐藏
    uint16_t label_bg_x; /// 标签背景 X 偏移
    uint16_t label_bg_y; /// 标签背景 Y 偏移
    uint16_t label_time_x; /// 时长 X 偏移
    uint16_t label_time_y; /// 时长 Y 偏移
    char time_text[16]; /// 已显示时长文本
} home_custom_label_info_t;

typedef struct {
    lv_obj_t *ip;
    res_img_info_t ip_info;
    uint16_t animation_index; /// 当前动画帧索引
    uint8_t initialized; /// 动画是否已初始化
} home_custom_animation_info_t;

typedef struct {
    lv_obj_t *title;
    lv_obj_t *icon;
    HOME_CUSTOM_MODE_SELECT_CAPSULE_MEMBERS
    HOME_CUSTOM_MODE_SELECT_CAROUSEL_MEMBERS
    uint8_t index; /// 模式索引
    uint8_t new_index; /// 新模式索引
    HOME_CUSTOM_MODE_SELECT_COMMON_MEMBERS
    HOME_CUSTOM_MODE_SELECT_LABEL_OFFSET_MEMBERS
    uint8_t mode_blink_state; /// 模式标题闪烁: 0: 不闪烁, 1: 闪烁.
    uint8_t icon_blink_state; /// 韵律图标闪烁: 0: 不闪烁, 1: 闪烁.
    uint32_t custom_mode_id; /// 个性化模式ID
    char custom_id_str[8]; /// 个性化模式ID字符串
} home_custom_mode_info_t;

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    home_custom_level_info_t level;
    home_custom_mode_info_t mode;
    home_custom_animation_info_t animation;
    home_custom_label_info_t label;
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t pump_status;
    uint8_t popup_ui_index;
    uint8_t force_update;
    uint8_t sub_ui_status;
} home_custom_ui_info_t;

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_custom_pump_mode_ui_remove(home_custom_ui_info_t *info);
void home_custom_pump_mode_ui_update(home_custom_ui_info_t *info, bool blink_visible,
    home_pump_mode_display_type_e display_type);

void home_custom_pump_level_ui_remove(home_custom_ui_info_t *info);
void home_custom_pump_level_ui_update(home_custom_ui_info_t *info, bool blink_visible);

void home_custom_pump_label_ui_remove(home_custom_ui_info_t *info);
void home_custom_pump_label_ui_update(home_custom_ui_info_t *info);
void home_custom_pump_label_apply_select_offset(home_custom_ui_info_t *info);

void home_custom_animation_ui_remove(home_custom_ui_info_t *info);
void home_custom_animation_ui_update(home_custom_ui_info_t *info);

void home_custom_blink_state_update(home_custom_ui_info_t *info, ui_proc_home_t *proc);

void home_custom_ui_prepare(ui_manager_context_t *thiz);
void home_custom_ui_remove(ui_manager_context_t *thiz);
int home_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __HOME_CUSTOM_UI_INTERNAL_H__
