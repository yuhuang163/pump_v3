#ifndef __HOME_COMMON_UI_H__
#define __HOME_COMMON_UI_H__

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
#include "home_ui.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_popup.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
/** App 个性化模式名称缓冲（与窗口层 HOME_CUSTOM_MODE_NAME_LEN 对齐） */
#ifndef HOME_MODE_NAME_LEN
#define HOME_MODE_NAME_LEN                            HOME_CUSTOM_MODE_NAME_LEN
#endif
/** 主页计时每位数字个数（MM / SS 各 2 位；字段在公共 info 中，仅系统主题绘制） */
#define HOME_MAIN_TIME_DIGIT_COUNT                    (2)
#define HOME_MAIN_TIME_COLON_COUNT                    (2)
/** 主页提示扫光宽度（像素，两主题共用） */
#define HOME_MAIN_HINT_SHINE_W                        (140)
/** 模式选择提示图：1=内嵌 BAJ 代码段（默认），0=资源包 MODE_SELECT_TIP（按主题取图） */
#ifndef HOME_MODE_SELECT_TIP_FROM_CODE
#define HOME_MODE_SELECT_TIP_FROM_CODE                (1)
#endif

/// 页间过渡 / 模式选择页参数（两主题共用，仅图片资源按主题取）
#ifndef HOME_PAGE_TRANSITION_MS
#define HOME_PAGE_TRANSITION_MS                       (400)
#endif
#ifndef HOME_PAGE_TRANSITION_HERO_MS
#define HOME_PAGE_TRANSITION_HERO_MS                  (120)
#endif
#ifndef HOME_PAGE_TRANSITION_HERO_SCALE_MIN
#define HOME_PAGE_TRANSITION_HERO_SCALE_MIN           ((256 * 85) / 100)
#endif
#ifndef HOME_PAGE_TRANSITION_SLIDE_PROGRESS
#if (HOME_PAGE_TRANSITION_MS > HOME_PAGE_TRANSITION_HERO_MS)
#define HOME_PAGE_TRANSITION_SLIDE_PROGRESS \
    (((HOME_PAGE_TRANSITION_MS - HOME_PAGE_TRANSITION_HERO_MS) * 256) / \
     HOME_PAGE_TRANSITION_MS)
#else
#define HOME_PAGE_TRANSITION_SLIDE_PROGRESS           (0)
#endif
#endif
#ifndef HOME_PAGE_TRANSITION_CIRCLE_SIZE
#define HOME_PAGE_TRANSITION_CIRCLE_SIZE              (260)
#endif
#ifndef HOME_PAGE_TRANSITION_CIRCLE_COLOR
#define HOME_PAGE_TRANSITION_CIRCLE_COLOR             (0x181818)
#endif
#ifndef HOME_PAGE_TRANSITION_CIRCLE_GAP
#define HOME_PAGE_TRANSITION_CIRCLE_GAP               (24)
#endif
#ifndef HOME_PAGE_TRANSITION_OPA_MIN
#define HOME_PAGE_TRANSITION_OPA_MIN                  (102)
#endif
#ifndef HOME_PAGE_TRANSITION_MAIN_BG_FULL_SIZE
#define HOME_PAGE_TRANSITION_MAIN_BG_FULL_SIZE        (360)
#endif

#define HOME_MODE_PAGE_DOT_MAX                        (6)

typedef struct home_base_ui_info home_base_ui_info_t;

typedef struct {
    void (*build)(home_base_ui_info_t *info);
    void (*update)(home_base_ui_info_t *info, ui_proc_home_t *proc);
    void (*remove)(home_base_ui_info_t *info);
    void (*transition_apply)(home_base_ui_info_t *info, int32_t hero_scale,
        int32_t bg_scale, bool transitioning);
} home_main_page_ops_t;

typedef struct {
    uint8_t left;
    uint8_t right;
} home_base_level_info_t;

typedef struct {
    uint8_t new_index;
    uint8_t custom_mode_active;
    uint32_t custom_mode_id;
    char custom_id_str[HOME_MODE_NAME_LEN];
} home_base_mode_info_t;

typedef struct {
    uint8_t paused_by_transition;
} home_base_animation_info_t;

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
} home_base_page_info_t;

typedef struct {
    lv_obj_t *bg;
    lv_obj_t *mode_title;
    lv_obj_t *time_h[HOME_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[HOME_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[HOME_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[HOME_MAIN_TIME_COLON_COUNT];
    lv_draw_buf_t *time_colon_buf;
    lv_obj_t *time_colon_2seg;
    lv_obj_t *hint_text;
    ui_shine_info_t hint_shine;
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;
    lv_obj_t *right_level;
    uint16_t shown_bg_id;
    uint32_t shown_hint_color;
    uint8_t shown_hint_low_bat;
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;
    uint8_t shown_speed;
    uint8_t shown_left_level;
    uint8_t shown_right_level;
    char shown_title[HOME_MODE_NAME_LEN];
} home_base_main_page_info_t;

typedef struct {
    lv_obj_t *dots[HOME_MODE_PAGE_DOT_MAX];
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    uint8_t tip_predecoded;
    uint8_t dot_count;
    uint8_t selected_dot;
    uint8_t ready;
} home_base_mode_chrome_info_t;

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
} home_base_mode_page_ctx_t;

struct home_base_ui_info {
    lv_obj_t *root;
    lv_obj_t *background;
    home_base_level_info_t level;
    home_base_mode_info_t mode;
    home_base_page_info_t page;
    home_base_animation_info_t animation;
    home_base_main_page_info_t main;
    home_base_mode_chrome_info_t mode_chrome;
    const home_main_page_ops_t *main_ops;
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t popup_ui_index;
    uint8_t force_update;
    uint8_t theme_type;
};

extern const home_main_page_ops_t home_base_main_page_ops;
extern const home_main_page_ops_t home_custom_main_page_ops;

#define HOME_UI_IS_CUSTOM_THEME(info) \
    ((info) != NULL && (info)->theme_type == RES_THEME_TYPE_CUSTOM)
#define HOME_UI_RES_THEME(info) \
    (HOME_UI_IS_CUSTOM_THEME(info) ? RES_THEME_TYPE_CUSTOM : RES_THEME_TYPE_BASE)

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
lv_obj_t *home_page_parent_get(const home_base_ui_info_t *info);
void home_ui_sync_work_mode(home_base_ui_info_t *info);
void home_page_ui_prepare(home_base_ui_info_t *info);
void home_page_ui_remove(home_base_ui_info_t *info);
void home_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
bool home_page_ui_is_transitioning(const home_base_ui_info_t *info);

void home_mode_page_ui_build(home_base_ui_info_t *info, uint8_t page_index);
void home_mode_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
void home_mode_page_ui_remove(home_base_ui_info_t *info);
void home_mode_page_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning);
void home_mode_chrome_prepare(home_base_ui_info_t *info);
void home_mode_chrome_remove(home_base_ui_info_t *info);
void home_mode_chrome_update(home_base_ui_info_t *info);

void home_base_animation_remove(home_base_ui_info_t *info);
void home_base_animation_pause(home_base_ui_info_t *info);
void home_base_animation_resume(home_base_ui_info_t *info);

#ifdef __cplusplus
}
#endif

#endif // __HOME_COMMON_UI_H__
