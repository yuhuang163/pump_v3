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
#include "pump_home_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define HOME_TIME_COLON_COUNT                      (2)
#define PUMP_HOME_TIME_DIGIT_COUNT                 (2)
typedef struct {
    lv_obj_t *arc;
    lv_obj_t *indicator;
    lv_obj_t *progress;
    lv_obj_t *progress_bg;
    uint8_t value;
    uint8_t new_value;
    uint8_t adjust_active; /// 0: 默认样式, 1: 调节样式.
} pump_home_base_level_t;

typedef struct {
    pump_home_base_level_t left;
    pump_home_base_level_t right;
    lv_obj_t *separator;
    uint8_t left_blink_state; /// 左力度闪烁: 0: 不闪烁, 1: 闪烁.
    uint8_t right_blink_state; /// 右力度闪烁: 0: 不闪烁, 1: 闪烁.
} pump_home_base_level_info_t;

typedef struct {
    lv_obj_t *title;
    lv_obj_t *mode;
    lv_obj_t *title_capsule;
    lv_obj_t *capsule_title;
    lv_obj_t *select_dots[3];
    uint8_t index;
    uint8_t new_index;
    uint8_t mode_type;
    uint8_t new_mode_type;
    uint8_t display_type;
    uint8_t select_ui_active;
    uint8_t select_capsule_style;
    uint8_t select_dot_count;
    uint8_t custom_mode_active;
    int16_t select_wave_y_offset;
    int16_t select_clock_y_offset;
    int16_t select_mode_y_offset;
    int16_t select_remain_y_offset;
    uint16_t mode_name_x;
    uint16_t mode_name_y;
    uint8_t mode_blink_state; /// 模式闪烁状态: 0: 不闪烁, 1: 闪烁.
    uint32_t custom_mode_id;
    char custom_id_str[8]; /// 个性化模式ID字符串
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
    uint16_t pending_anim_start; /// 转场待切换起始ID
    uint16_t pending_anim_end; /// 转场待切换结束ID
    uint16_t shown_frame_res_id; /// 已显示帧资源ID
    uint8_t anim_phase; /// 动画阶段
    uint8_t anim_play_state; /// 动画播放状态: 0: 播放, 1: 暂停.
    uint32_t transition_start_ms; /// 转场开始时间
    uint32_t last_frame_tick_ms; /// 上次切帧时间
} pump_home_base_animation_info_t;

typedef struct {
    lv_obj_t *h[PUMP_HOME_TIME_DIGIT_COUNT];
    lv_obj_t *m[PUMP_HOME_TIME_DIGIT_COUNT];
    lv_obj_t *s[PUMP_HOME_TIME_DIGIT_COUNT];
    lv_obj_t *colon[HOME_TIME_COLON_COUNT];
    lv_obj_t *remain_minus;
    lv_obj_t *remain_m[PUMP_HOME_TIME_DIGIT_COUNT];
    lv_obj_t *remain_colon;
    lv_obj_t *remain_s[PUMP_HOME_TIME_DIGIT_COUNT];
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t digit_size; /// 时数字体规格：PUMP_TIME_2SEG / PUMP_TIME_3SEG
    int8_t y_offset;
    uint8_t time_blink_state; /// 时间闪烁状态: 0: 不闪烁, 1: 闪烁.
    uint32_t remain_time;
    uint32_t remain_time_shown;
    uint8_t force_realign;
} pump_home_base_time_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    pump_home_base_mode_info_t mode;
    pump_home_base_level_info_t level;
    pump_home_base_time_t time;
    pump_home_base_animation_info_t animation;
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t force_update;
    uint8_t popup_ui_index;
} pump_home_base_ui_info_t;

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void pump_home_base_mode_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_base_mode_ui_update(pump_home_base_ui_info_t *info, bool blink_visible,
    pump_mode_display_type_e display_type);
void pump_home_base_select_offset_apply(pump_home_base_ui_info_t *info);

void pump_home_base_level_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_base_level_ui_update(pump_home_base_ui_info_t *info, bool blink_visible);

void pump_home_base_time_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_base_time_ui_update(pump_home_base_ui_info_t *info, uint16_t hold_time, bool blink_visible);
void pump_home_base_time_apply_select_offset(pump_home_base_ui_info_t *info);

void pump_home_base_animation_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_base_animation_ui_update(pump_home_base_ui_info_t *info);

void pump_home_base_blink_state_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);

void pump_home_base_ui_prepare(ui_manager_context_t *thiz);
void pump_home_base_ui_remove(ui_manager_context_t *thiz);
int pump_home_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_HOME_BASE_UI_INTERNAL_H__
