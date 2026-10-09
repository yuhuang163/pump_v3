#ifndef __PUMP_HOME_COMMON_UI_H__
#define __PUMP_HOME_COMMON_UI_H__

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
#include "battery_ui.h" // IWYU pragma: keep
#include "pump_home_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_popup.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_BASE_TIME_3SEG_TEST                   (0)

/** App 个性化模式名称缓冲长度（含 '\0'，UI 侧略大于存储上限） */
#define PUMP_HOME_BASE_CUSTOM_MODE_NAME_LEN              (32)

/** 离 page0 进入模式选择：时间胶囊淡入 + 上移；默认开启 */
#ifndef IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
#define IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM             (1)
#endif
#ifndef IS_SUPPORT_PUMP_SEAL_FAIL_ANIM
#define IS_SUPPORT_PUMP_SEAL_FAIL_ANIM                   (0) /// 1: 独立失败动画；0: 沿用检测中只换文案
#endif

#define PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT             (2)
#define PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT             (2)
#define PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT             (2)
#define PUMP_HOME_BASE_MODE_TIME_COLON_COUNT             (2)

/** 主页提示扫光宽度（像素，CPU 软边剖面） */
#define PUMP_HOME_BASE_MAIN_HINT_SHINE_W                 (140)
/** 漏气检测/结果文案缓冲长度（含 '\0'） */
#define PUMP_HOME_MAIN_SEAL_TIP_TEXT_LEN                 (80)

/// 页间过渡动画参数（与 home 一致，两主题共用）
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_MS
#define PUMP_HOME_BASE_PAGE_TRANSITION_MS                (400)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_HERO_MS
#define PUMP_HOME_BASE_PAGE_TRANSITION_HERO_MS           (120)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_HERO_SCALE_MIN
#define PUMP_HOME_BASE_PAGE_TRANSITION_HERO_SCALE_MIN    ((256 * 85) / 100)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_SLIDE_PROGRESS
#if (PUMP_HOME_BASE_PAGE_TRANSITION_MS > PUMP_HOME_BASE_PAGE_TRANSITION_HERO_MS)
#define PUMP_HOME_BASE_PAGE_TRANSITION_SLIDE_PROGRESS \
    (((PUMP_HOME_BASE_PAGE_TRANSITION_MS - PUMP_HOME_BASE_PAGE_TRANSITION_HERO_MS) * 256) / \
     PUMP_HOME_BASE_PAGE_TRANSITION_MS)
#else
#define PUMP_HOME_BASE_PAGE_TRANSITION_SLIDE_PROGRESS    (0)
#endif
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE
#define PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE       (260)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR
#define PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR      (0x181818)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_GAP
#define PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_GAP        (24)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_OPA_MIN
#define PUMP_HOME_BASE_PAGE_TRANSITION_OPA_MIN           (102)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_MAIN_BG_FULL_SIZE
#define PUMP_HOME_BASE_PAGE_TRANSITION_MAIN_BG_FULL_SIZE (360)
#endif

typedef struct {
    uint8_t left;
    uint8_t right;
} pump_home_base_level_info_t;

typedef struct {
    uint8_t new_index;
    uint8_t new_mode_type;
    uint8_t custom_mode_active;
    uint32_t custom_mode_id;
    char custom_id_str[PUMP_HOME_BASE_CUSTOM_MODE_NAME_LEN];
} pump_home_base_mode_info_t;

typedef enum {
    PUMP_HOME_BASE_ANIM_STIMULATION_WAVE = 0,
    PUMP_HOME_BASE_ANIM_EXPRESSION_WAVE,
    PUMP_HOME_BASE_ANIM_MIX_WAVE,
    PUMP_HOME_BASE_ANIM_PROGRAM_STIMULATION_WAVE,
    PUMP_HOME_BASE_ANIM_PROGRAM_EXPRESSION_WAVE,
    PUMP_HOME_BASE_ANIM_PROGRAM_RELAX_WAVE,
    PUMP_HOME_BASE_ANIM_CHECKING_SEAL,
    PUMP_HOME_BASE_ANIM_CHECK_PASSED,
    PUMP_HOME_BASE_ANIM_CHECK_FAILED,
    PUMP_HOME_BASE_ANIM_MAX,
} pump_home_base_anim_res_type_e;

typedef struct {
    lv_obj_t *wave;
    res_img_info_t wave_info;
    uint8_t wave_res_type;
    uint16_t frame_res_id;
    uint16_t frame_end_res_id;
    uint16_t current_anim_start;
    uint16_t shown_frame_res_id;
    uint8_t anim_play_state;
    uint8_t anim_loop;
    uint8_t paused_by_transition;
    uint8_t hidden_by_mode_switch;
    uint8_t seal_tip;
    uint32_t last_frame_tick_ms;
} pump_home_base_animation_info_t;

typedef struct {
    lv_obj_t *page;
    lv_obj_t *peer_page;
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t transitioning;
    uint8_t enter_instant;
    int8_t slide_dir;
    int16_t progress;
    uint32_t start_ms;
} pump_home_base_page_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *mode_title;
    lv_obj_t *time_h[PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT];
    lv_draw_buf_t *time_colon_buf;
    lv_obj_t *time_colon_2seg;
    lv_obj_t *play_icon;
    lv_obj_t *hint_text;
    ui_shine_info_t hint_shine;
    lv_obj_t *mode_switch_label;
    lv_obj_t *seal_check_label;
    ui_shine_info_t seal_check_shine;
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;
    lv_obj_t *right_level;
    uint16_t shown_bg_id;
    uint32_t shown_hint_color;
    uint32_t shown_time_color;
    uint16_t shown_play_icon_id;
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;
    uint8_t shown_speed;
    uint8_t shown_left_level;
    uint8_t shown_right_level;
    uint8_t shown_paused;
    uint8_t shown_mode_switch;
    uint8_t shown_seal_check;
    uint8_t shown_seal_keep_time;
    uint32_t shown_seal_color;
    uint8_t tracked_mode_type;
    uint8_t tracked_pump_mode;
    uint8_t tip_mode_type;
    uint32_t mode_switch_time;
    char shown_title[PUMP_HOME_BASE_CUSTOM_MODE_NAME_LEN];
    char shown_seal_text[PUMP_HOME_MAIN_SEAL_TIP_TEXT_LEN];
#if IS_SUPPORT_PUMP_SEAL_RESULT_TIP
    lv_obj_t *wear_result_text;
    uint8_t wear_result[AIR_PRESSURE_NUM];
    uint8_t seal_result;
#endif
} pump_home_base_main_page_info_t;

#define PUMP_HOME_BASE_MODE_PAGE_DOT_MAX                     (6)

typedef struct {
    lv_obj_t *dots[PUMP_HOME_BASE_MODE_PAGE_DOT_MAX];
    lv_obj_t *time_label;
    lv_obj_t *status_icon;
    lv_obj_t *time_h[PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[PUMP_HOME_BASE_MODE_TIME_COLON_COUNT];
    uint8_t dot_count;
    uint8_t selected_dot;
    uint8_t ready;
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;
    uint8_t shown_paused;
    uint16_t shown_status_icon_id;
#if IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
    uint8_t time_shown;
    uint8_t time_enter_animating;
#endif
} pump_home_base_mode_chrome_info_t;

typedef struct {
    lv_obj_t *circle_bg;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *duration;
    uint8_t page_index;
    uint8_t mode;
    uint16_t icon_id;
    uint8_t icon_predecoded;
    uint8_t title_ellipsis;
    char shown_title[33];
    char shown_duration[16];
} pump_home_base_mode_page_ctx_t;

typedef struct pump_home_base_ui_info pump_home_base_ui_info_t;

typedef struct {
    void (*build)(pump_home_base_ui_info_t *info);
    void (*update)(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);
    void (*remove)(pump_home_base_ui_info_t *info);
    void (*transition_apply)(pump_home_base_ui_info_t *info, int32_t hero_scale,
        int32_t bg_scale, bool transitioning);
} pump_home_main_page_ops_t;

struct pump_home_base_ui_info {
    lv_obj_t *root;
    lv_obj_t *background;
    pump_home_base_level_info_t level;
    pump_home_base_mode_info_t mode;
    pump_home_base_page_info_t page;
    pump_home_base_animation_info_t animation;
    pump_home_base_main_page_info_t main;
    pump_home_base_mode_chrome_info_t mode_chrome;
    const pump_home_main_page_ops_t *main_ops;
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t popup_ui_index;
    uint8_t force_update;
    uint8_t theme_type;
#if IS_SUPPORT_BASE_TIME_3SEG_TEST
    uint8_t test_hours;
#endif
};

#define PUMP_HOME_UI_IS_CUSTOM_THEME(info) \
    ((info) != NULL && (info)->theme_type == RES_THEME_TYPE_CUSTOM)
#define PUMP_HOME_UI_RES_THEME(info) \
    (PUMP_HOME_UI_IS_CUSTOM_THEME(info) ? RES_THEME_TYPE_CUSTOM : RES_THEME_TYPE_BASE)

extern const pump_home_main_page_ops_t pump_home_base_main_page_ops;
extern const pump_home_main_page_ops_t pump_home_custom_main_page_ops;

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

void pump_home_mode_page_ui_build(pump_home_base_ui_info_t *info, uint8_t page_index);
void pump_home_mode_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);
void pump_home_mode_page_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_mode_page_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning);
void pump_home_mode_chrome_prepare(pump_home_base_ui_info_t *info);
void pump_home_mode_chrome_remove(pump_home_base_ui_info_t *info);
void pump_home_mode_chrome_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);

void pump_home_base_animation_ui_remove(pump_home_base_ui_info_t *info);
void pump_home_base_animation_ui_update(pump_home_base_ui_info_t *info);
void pump_home_base_animation_pause(pump_home_base_ui_info_t *info);
void pump_home_base_animation_resume(pump_home_base_ui_info_t *info);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_HOME_COMMON_UI_H__
