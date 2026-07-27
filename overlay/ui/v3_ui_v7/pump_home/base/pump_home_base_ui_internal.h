#ifndef __PUMP_HOME_BASE_UI_INTERNAL_H__
#define __PUMP_HOME_BASE_UI_INTERNAL_H__

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
#include "pump_home_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_BASE_PUMP_HOME_COMP_BG           (0)
#define PUMP_HOME_MAIN_TIME_DIGIT_COUNT             (2)

typedef struct {
    uint8_t left;  /// 左力度
    uint8_t right; /// 右力度
} pump_home_base_level_info_t;

typedef struct {
    uint8_t new_index; /// 当前泵奶模式
    uint8_t new_mode_type; /// 当前运行韵律类型
    uint8_t custom_mode_active; /// 个性化模式是否激活
    uint32_t custom_mode_id; /// 个性化模式 ID
    char custom_id_str[8]; /// 个性化模式 ID 字符串
} pump_home_base_mode_info_t;

typedef enum {
    PUMP_HOME_BASE_ANIM_STIMULATION_WAVE = 0,
    PUMP_HOME_BASE_ANIM_LACTATION_WAVE,
    PUMP_HOME_BASE_ANIM_MIX_WAVE,
    PUMP_HOME_BASE_ANIM_RELAX_WAVE,
    PUMP_HOME_BASE_ANIM_MAX,
} pump_home_base_anim_res_type_e;

typedef struct {
    lv_obj_t *wave;
    res_img_info_t wave_info;
    uint8_t wave_res_type; /// 当前动画资源类型
    uint16_t frame_res_id; /// 当前帧资源ID
    uint16_t frame_end_res_id; /// 末帧资源ID
    uint16_t current_anim_start; /// 当前动画起始ID
    uint16_t shown_frame_res_id; /// 已显示帧资源ID
    uint8_t anim_play_state; /// 动画播放状态: 0: 播放, 1: 暂停.
    uint8_t paused_by_transition; /// 过渡期间暂停波形动画
    uint32_t last_frame_tick_ms; /// 上次切帧时间
} pump_home_base_animation_info_t;

typedef struct {
    lv_obj_t *page;
    lv_obj_t *peer_page;
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t transitioning;
    int8_t slide_dir;
    int16_t progress;
    uint32_t start_ms;
} pump_home_base_page_info_t;

typedef struct {
    lv_obj_t *bg;                       /// 模式背景 PNG
    lv_obj_t *mode_title;               /// 模式文案
    lv_obj_t *time_m[PUMP_HOME_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_HOME_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon;               /// 冒号 BAJ
    lv_obj_t *play_icon;                /// 暂停态播放图标
    lv_obj_t *hint_text;                /// Press to resume
    lv_obj_t *hint_shine;               /// 扫光亮色文案（裁剪窗内）
    lv_obj_t *hint_mask;                /// 扫光裁剪窗
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
    uint8_t shown_paused;               /// 当前是否显示暂停态
    int16_t mask_x;
    uint32_t mask_tick_ms;
    char shown_title[24];
} pump_home_base_main_page_info_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    pump_home_base_level_info_t level;
    pump_home_base_mode_info_t mode;
    pump_home_base_page_info_t page;
    pump_home_base_animation_info_t animation;
    pump_home_base_main_page_info_t main;
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t popup_ui_index;
    uint8_t force_update;
} pump_home_base_ui_info_t;

static inline lv_obj_t *pump_home_page_parent_get(const pump_home_base_ui_info_t *info)
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
void pump_home_page_ui_prepare(pump_home_base_ui_info_t *info);
void pump_home_page_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);
bool pump_home_page_ui_is_transitioning(const pump_home_base_ui_info_t *info);
void pump_home_page_ui_request_switch(pump_home_base_ui_info_t *info, pump_home_page_slide_dir_e dir);

void pump_home_main_page_ui_build(pump_home_base_ui_info_t *info);
void pump_home_main_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);
void pump_home_main_page_ui_remove(pump_home_base_ui_info_t *info);

void pump_home_base_animation_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_base_animation_ui_update(pump_home_base_ui_info_t *info);
void pump_home_base_animation_pause(pump_home_base_ui_info_t *info);
void pump_home_base_animation_resume(pump_home_base_ui_info_t *info);

void pump_home_base_ui_prepare(ui_manager_context_t *thiz);
void pump_home_base_ui_remove(ui_manager_context_t *thiz);
int pump_home_base_ui_process(ui_manager_context_t *thiz);
void pump_home_base_ui_page_switch(pump_home_page_slide_dir_e dir);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_HOME_BASE_UI_INTERNAL_H__
