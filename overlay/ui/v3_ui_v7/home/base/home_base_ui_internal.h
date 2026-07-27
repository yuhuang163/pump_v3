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
#include "ui_img_resource.h" // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep
#include "home_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_BASE_HOME_COMP_BG            (0)
#define HOME_MAIN_TIME_DIGIT_COUNT              (2)
/** 扫光软边过渡：1 开启（蒙层 A8），0 关闭（硬边亮色扫光，默认） */
#ifndef HOME_MAIN_HINT_SHINE_SOFT_ENABLE
#define HOME_MAIN_HINT_SHINE_SOFT_ENABLE        (0)
#endif

#if HOME_MAIN_HINT_SHINE_SOFT_ENABLE
struct _lv_draw_buf_t;
typedef struct _lv_draw_buf_t lv_draw_buf_t;
#endif

typedef struct {
    uint8_t left;  /// 左力度
    uint8_t right; /// 右力度
} home_base_level_info_t;

typedef struct {
    uint8_t new_index; /// 当前泵奶模式
    uint8_t custom_mode_active; /// 个性化模式是否激活
    uint32_t custom_mode_id; /// 个性化模式 ID
    char custom_id_str[8]; /// 个性化模式 ID 字符串
} home_base_mode_info_t;

typedef struct {
    uint8_t paused_by_transition; /// 过渡期间暂停首页蒙层动画
} home_base_animation_info_t;

typedef struct {
    lv_obj_t *page;
    lv_obj_t *peer_page;
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t transitioning;
    int8_t slide_dir;
    int16_t progress;
    uint32_t start_ms;
} home_base_page_info_t;

typedef struct {
    lv_obj_t *bg;                       /// 模式背景 PNG
    lv_obj_t *mode_title;               /// 模式文案
    lv_obj_t *time_m[HOME_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[HOME_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon;               /// 冒号 BAJ
    lv_obj_t *hint_text;                /// Press to start pumping
    lv_obj_t *hint_shine;               /// 扫光亮色文案（裁剪窗内）
    lv_obj_t *hint_mask;                /// 扫光裁剪窗
#if HOME_MAIN_HINT_SHINE_SOFT_ENABLE
    lv_draw_buf_t *hint_mask_a8;        /// 软边 A8（由蒙层 PNG alpha 生成）
#endif
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;               /// Lx
    lv_obj_t *right_level;              /// Rx
    uint16_t shown_bg_id;
    uint32_t shown_hint_color;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_speed;
    uint8_t shown_left_level;
    uint8_t shown_right_level;
    int16_t mask_x;
    uint32_t mask_tick_ms;
    char shown_title[24];
} home_base_main_page_info_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    home_base_level_info_t level;
    home_base_mode_info_t mode;
    home_base_page_info_t page;
    home_base_animation_info_t animation;
    home_base_main_page_info_t main;
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t popup_ui_index;
    uint8_t force_update;
} home_base_ui_info_t;

static inline lv_obj_t *home_page_parent_get(const home_base_ui_info_t *info)
{
    if (info != NULL && info->page.page != NULL && lv_obj_is_valid(info->page.page)) {
        return info->page.page;
    }
    return info != NULL ? info->root : NULL;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void home_page_ui_prepare(home_base_ui_info_t *info);
void home_page_ui_remove(home_base_ui_info_t *info);
void home_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
bool home_page_ui_is_transitioning(const home_base_ui_info_t *info);
void home_page_ui_request_switch(home_base_ui_info_t *info, home_page_slide_dir_e dir);

void home_main_page_ui_build(home_base_ui_info_t *info);
void home_main_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
void home_main_page_ui_remove(home_base_ui_info_t *info);

void home_base_animation_remove(home_base_ui_info_t *info);
void home_base_animation_pause(home_base_ui_info_t *info);
void home_base_animation_resume(home_base_ui_info_t *info);

void home_base_ui_prepare(ui_manager_context_t *thiz);
void home_base_ui_remove(ui_manager_context_t *thiz);
int home_base_ui_process(ui_manager_context_t *thiz);
void home_base_ui_page_switch(home_page_slide_dir_e dir);

#ifdef __cplusplus
}
#endif

#endif // __HOME_BASE_UI_INTERNAL_H__
