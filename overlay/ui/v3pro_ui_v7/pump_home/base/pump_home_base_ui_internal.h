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
#include "battery_ui.h" // IWYU pragma: keep
#include "pump_home_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_BASE_PUMP_HOME_COMP_BG                (0)
#define IS_SUPPORT_BASE_TIME_3SEG_TEST                   (0)

/** App 个性化模式名称缓冲长度（含 '\0'，UI 侧略大于存储上限） */
#define PUMP_HOME_BASE_CUSTOM_MODE_NAME_LEN              (24)

/** 离 page0 进入模式选择：时间胶囊淡入 + 上移；默认开启 */
#ifndef IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
#define IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM             (1)
#endif


#define PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT             (2)
#define PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT             (2)
#define PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT             (2)
#define PUMP_HOME_BASE_MODE_TIME_COLON_COUNT             (2)

/** 主页提示扫光宽度（像素，CPU 软边剖面） */
#define PUMP_HOME_BASE_MAIN_HINT_SHINE_W                 (140)

/// 页间过渡动画参数（与 home base 一致）
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_MS
#define PUMP_HOME_BASE_PAGE_TRANSITION_MS                (400)
#endif
/// ICON 缩放 + 标题字号（36↔29）切换时长
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_HERO_MS
#define PUMP_HOME_BASE_PAGE_TRANSITION_HERO_MS           (120)
#endif
/// ICON 最小缩放（256=100%，218≈85%）
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_HERO_SCALE_MIN
#define PUMP_HOME_BASE_PAGE_TRANSITION_HERO_SCALE_MIN    ((256 * 85) / 100)
#endif
/// 位移阶段进度 /256；默认按总时长扣除 HERO_MS 推导
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
#define PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE       (260)   /// 过渡圆直径
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR
#define PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR      (0x181818)
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_GAP
#define PUMP_HOME_BASE_PAGE_TRANSITION_CIRCLE_GAP        (24)    /// 两圆边缘间距
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_OPA_MIN
#define PUMP_HOME_BASE_PAGE_TRANSITION_OPA_MIN           (102)   /// ~40%
#endif
#ifndef PUMP_HOME_BASE_PAGE_TRANSITION_MAIN_BG_FULL_SIZE
#define PUMP_HOME_BASE_PAGE_TRANSITION_MAIN_BG_FULL_SIZE (360)   /// page0 全尺寸，过渡不缩放
#endif

typedef struct {
    uint8_t left;  /// 左力度
    uint8_t right; /// 右力度
} pump_home_base_level_info_t;

typedef struct {
    uint8_t new_index; /// 当前泵奶模式
    uint8_t new_mode_type; /// 当前运行韵律类型
    uint8_t custom_mode_active; /// 个性化模式是否激活
    uint32_t custom_mode_id; /// 个性化模式 ID
    char custom_id_str[PUMP_HOME_BASE_CUSTOM_MODE_NAME_LEN]; /// App 下发的个性化模式名称（已含展示前缀）
} pump_home_base_mode_info_t;

typedef enum {
    PUMP_HOME_BASE_ANIM_STIMULATION_WAVE = 0,
    PUMP_HOME_BASE_ANIM_EXPRESSION_WAVE,
    PUMP_HOME_BASE_ANIM_MIX_WAVE,
    PUMP_HOME_BASE_ANIM_PROGRAM_STIMULATION_WAVE,
    PUMP_HOME_BASE_ANIM_PROGRAM_EXPRESSION_WAVE,
    PUMP_HOME_BASE_ANIM_PROGRAM_RELAX_WAVE,
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
    uint8_t hidden_by_mode_switch; /// 子模式切换提醒期间隐藏并停止波形
    uint32_t last_frame_tick_ms; /// 上次切帧时间
} pump_home_base_animation_info_t;

typedef struct {
    lv_obj_t *page;
    lv_obj_t *peer_page;
    uint8_t page_index;
    uint8_t shown_page_index;
    uint8_t transitioning;
    uint8_t enter_instant; /// 从非首页 UI 进入：首帧按 page_index 直达，不做过渡
    int8_t slide_dir;
    int16_t progress;
    uint32_t start_ms;
} pump_home_base_page_info_t;

typedef struct {
    lv_obj_t *bg;                       /// 模式背景 PNG
    lv_obj_t *mode_title;               /// 模式文案
    lv_obj_t *time_h[PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[PUMP_HOME_BASE_MAIN_TIME_COLON_COUNT]; /// 三段：HH-MM / MM-SS 冒号 BAJ
    lv_draw_buf_t *time_colon_buf;      /// HOUR_COLON 预解码（两冒号共用）
    lv_obj_t *time_colon_2seg;          /// 两段：居中 CLOCK_COLON
    lv_obj_t *play_icon;                /// 暂停态播放图标
    lv_obj_t *hint_text;                /// Press to resume
    ui_shine_info_t hint_shine;         /// 扫光（字库文案软边）
    lv_obj_t *mode_switch_label;        /// 子模式切换提醒文案
    lv_obj_t *speed_icon;
    lv_obj_t *speed_value;
    lv_obj_t *left_level;               /// Lx
    lv_obj_t *right_level;              /// Rx
    uint16_t shown_bg_id;
    uint32_t shown_hint_color;
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;           /// 2=MM:SS / 3=HH:MM:SS
    uint8_t shown_speed;
    uint8_t shown_left_level;
    uint8_t shown_right_level;
    uint8_t shown_paused;               /// 当前是否显示暂停态
    uint8_t shown_mode_switch;          /// 当前是否显示子模式切换提醒
    uint8_t tracked_mode_type;          /// 用于检测子模式切换
    uint8_t tracked_pump_mode;          /// 用于区分韵律内子模式切换 / 工作模式切换
    uint8_t tip_mode_type;              /// 提醒文案对应的子模式
    uint32_t mode_switch_time;          /// 子模式切换提醒起始时间
    char shown_title[PUMP_HOME_BASE_CUSTOM_MODE_NAME_LEN];
} pump_home_base_main_page_info_t;

/// 模式选择页固定层（挂 root，不参与切页位移动画；无文案提示/扫光）
#define PUMP_HOME_BASE_MODE_PAGE_DOT_MAX                     (6)

typedef struct {
    lv_obj_t *dots[PUMP_HOME_BASE_MODE_PAGE_DOT_MAX];
    lv_obj_t *time_label;               /// 时间背景 BAJ
    lv_obj_t *status_icon;              /// 泵奶中暂停 / 暂停态播放
    lv_obj_t *time_h[PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT];
    lv_obj_t *time_m[PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[PUMP_HOME_BASE_MODE_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon[PUMP_HOME_BASE_MODE_TIME_COLON_COUNT]; /// [0]HH-MM [1]MM-SS 字库冒号
    uint8_t dot_count;
    uint8_t selected_dot;
    uint8_t ready;
    uint8_t shown_hours;
    uint8_t shown_minutes;
    uint8_t shown_seconds;
    uint8_t shown_digit_size;           /// 2=MM:SS / 3=HH:MM:SS
    uint8_t shown_paused;
    uint16_t shown_status_icon_id;
#if IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM
    uint8_t time_shown;                 /// 时间层当前是否可见（入场动效用）
    uint8_t time_enter_animating;       /// 离 page0 入场位移动画中
#endif
} pump_home_base_mode_chrome_info_t;

/// 模式/韵律选择页控件（挂在 page user_data，避免切页过渡覆盖）
typedef struct {
    lv_obj_t *circle_bg;                /// 切换动画圆背景（不缩放）
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_obj_t *duration;                 /// 韵律总时间（Milk Boost / Power Pumping / Custom）
    uint8_t page_index;
    uint8_t mode;
    uint16_t icon_id;
    uint8_t icon_predecoded;
    uint8_t title_ellipsis;             /// 原文超 32 字符，展示需 "..."
    char shown_title[33];               /// 最多 32 字符 + '\0'
    char shown_duration[16];
} pump_home_base_mode_page_ctx_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    pump_home_base_level_info_t level;
    pump_home_base_mode_info_t mode;
    pump_home_base_page_info_t page;
    pump_home_base_animation_info_t animation;
    pump_home_base_main_page_info_t main;
    pump_home_base_mode_chrome_info_t mode_chrome; /// 定位点 + 泵奶时间（root 固定层）
    storage_manager_context_t *st;
    ui_manager_context_t *ui;
    uint8_t popup_ui_index;
    uint8_t force_update;
#if IS_SUPPORT_BASE_TIME_3SEG_TEST
    uint8_t test_hours;
#endif
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
/// 切换动画：page0 不缩放仅位移；模式页由 circle + hero 处理
void pump_home_main_page_transition_apply(pump_home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning);

void pump_home_mode_page_ui_build(pump_home_base_ui_info_t *info, uint8_t page_index);
void pump_home_mode_page_ui_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);
void pump_home_mode_page_ui_remove(pump_home_base_ui_info_t *info);
/// 切换动画：圆背景显隐 + ICON/模式名缩放（圆不参与缩放）
void pump_home_mode_page_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning);
/// 定位点/时间固定层（root）：page0 隐藏；离 page0 动画起显示；
/// 时间入场位移由 IS_SUPPORT_PUMP_MODE_TIME_ENTER_ANIM 控制
void pump_home_mode_chrome_prepare(pump_home_base_ui_info_t *info);
void pump_home_mode_chrome_remove(pump_home_base_ui_info_t *info);
void pump_home_mode_chrome_update(pump_home_base_ui_info_t *info, ui_proc_pump_t *proc);

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
