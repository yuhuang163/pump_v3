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
#define IS_SUPPORT_BASE_HOME_COMP_BG                 (0)
#define HOME_BASE_MAIN_TIME_DIGIT_COUNT              (2)
/** 主页提示扫光方式（三选一） */
#define HOME_BASE_HINT_SHINE_HARD                    (0) /// 硬边裁切亮色文案
#define HOME_BASE_HINT_SHINE_SOFT                    (1) /// A8 软边（PNG alpha × 抛物线）
#define HOME_BASE_HINT_SHINE_OVERLAY                 (2) /// 图片蒙层（直接用 TEXT_SHINE_MASK alpha）
#ifndef HOME_BASE_MAIN_HINT_SHINE_MODE
#define HOME_BASE_MAIN_HINT_SHINE_MODE               HOME_BASE_HINT_SHINE_OVERLAY
#endif
#define HOME_BASE_MAIN_HINT_SHINE_IS_HARD \
    (HOME_BASE_MAIN_HINT_SHINE_MODE == HOME_BASE_HINT_SHINE_HARD)
#define HOME_BASE_MAIN_HINT_SHINE_IS_SOFT \
    (HOME_BASE_MAIN_HINT_SHINE_MODE == HOME_BASE_HINT_SHINE_SOFT)
#define HOME_BASE_MAIN_HINT_SHINE_IS_OVERLAY \
    (HOME_BASE_MAIN_HINT_SHINE_MODE == HOME_BASE_HINT_SHINE_OVERLAY)
/** SOFT/OVERLAY：使用 TEXT_SHINE_MASK 做 bitmap 蒙层 */
#define HOME_BASE_MAIN_HINT_SHINE_USE_IMG_MASK \
    (HOME_BASE_MAIN_HINT_SHINE_IS_SOFT || HOME_BASE_MAIN_HINT_SHINE_IS_OVERLAY)
/** 兼容旧宏：1=软边 A8，0=非软边 */
#ifndef HOME_BASE_MAIN_HINT_SHINE_SOFT_ENABLE
#define HOME_BASE_MAIN_HINT_SHINE_SOFT_ENABLE        HOME_BASE_MAIN_HINT_SHINE_IS_SOFT
#endif
/**
 * 主页扫光宽度（像素）。改此宏即可控制光带宽窄。
 * HARD 默认 80；SOFT/OVERLAY 默认 140（与 TEXT_SHINE_MASK 同宽，可小于资源图做居中裁切）。
 */
#ifndef HOME_BASE_MAIN_HINT_SHINE_W
#if HOME_BASE_MAIN_HINT_SHINE_IS_HARD
#define HOME_BASE_MAIN_HINT_SHINE_W                  (80)
#else
#define HOME_BASE_MAIN_HINT_SHINE_W                  (140)
#endif
#endif

/** 模式选择提示扫光方式（三选一，与主页共用 HARD/SOFT/OVERLAY 取值） */
#ifndef HOME_BASE_MODE_TIP_SHINE_MODE
#define HOME_BASE_MODE_TIP_SHINE_MODE                HOME_BASE_HINT_SHINE_OVERLAY
#endif
#define HOME_BASE_MODE_TIP_SHINE_IS_HARD \
    (HOME_BASE_MODE_TIP_SHINE_MODE == HOME_BASE_HINT_SHINE_HARD)
#define HOME_BASE_MODE_TIP_SHINE_IS_SOFT \
    (HOME_BASE_MODE_TIP_SHINE_MODE == HOME_BASE_HINT_SHINE_SOFT)
#define HOME_BASE_MODE_TIP_SHINE_IS_OVERLAY \
    (HOME_BASE_MODE_TIP_SHINE_MODE == HOME_BASE_HINT_SHINE_OVERLAY)
#define HOME_BASE_MODE_TIP_SHINE_USE_IMG_MASK \
    (HOME_BASE_MODE_TIP_SHINE_IS_SOFT || HOME_BASE_MODE_TIP_SHINE_IS_OVERLAY)
/** 兼容旧宏：1=软边 A8，0=硬边裁切 */
#ifndef HOME_BASE_MODE_TIP_SHINE_SOFT_ENABLE
#define HOME_BASE_MODE_TIP_SHINE_SOFT_ENABLE         HOME_BASE_MODE_TIP_SHINE_IS_SOFT
#endif
/**
 * 模式页 tip 扫光宽度（像素）。改此宏即可控制光带宽窄。
 * HARD 默认 80；SOFT/OVERLAY 默认 140。
 */
#ifndef HOME_BASE_MODE_TIP_SHINE_W
#if HOME_BASE_MODE_TIP_SHINE_IS_HARD
#define HOME_BASE_MODE_TIP_SHINE_W                   (80)
#else
#define HOME_BASE_MODE_TIP_SHINE_W                   (80)//(140)
#endif
#endif
/** 模式选择提示图：1=内嵌 BAJ 代码段（默认），0=资源包 BASE_RES_ID_MODE_SELECT_TIP */
#ifndef HOME_BASE_MODE_SELECT_TIP_FROM_CODE
#define HOME_BASE_MODE_SELECT_TIP_FROM_CODE          (1)
#endif

/// 页间过渡动画参数（base 主题）
#ifndef HOME_BASE_PAGE_TRANSITION_MS
#define HOME_BASE_PAGE_TRANSITION_MS                 (400)
#endif
/// ICON 缩放 + 标题字号（36↔29）切换时长
#ifndef HOME_BASE_PAGE_TRANSITION_HERO_MS
#define HOME_BASE_PAGE_TRANSITION_HERO_MS            (120)
#endif
/// ICON 最小缩放（256=100%，218≈85%）
#ifndef HOME_BASE_PAGE_TRANSITION_HERO_SCALE_MIN
#define HOME_BASE_PAGE_TRANSITION_HERO_SCALE_MIN     ((256 * 85) / 100)
#endif
/// 位移阶段进度 /256；默认按总时长扣除 HERO_MS 推导
#ifndef HOME_BASE_PAGE_TRANSITION_SLIDE_PROGRESS
#if (HOME_BASE_PAGE_TRANSITION_MS > HOME_BASE_PAGE_TRANSITION_HERO_MS)
#define HOME_BASE_PAGE_TRANSITION_SLIDE_PROGRESS \
    (((HOME_BASE_PAGE_TRANSITION_MS - HOME_BASE_PAGE_TRANSITION_HERO_MS) * 256) / \
     HOME_BASE_PAGE_TRANSITION_MS)
#else
#define HOME_BASE_PAGE_TRANSITION_SLIDE_PROGRESS     (0)
#endif
#endif
#ifndef HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE
#define HOME_BASE_PAGE_TRANSITION_CIRCLE_SIZE        (260)   /// 过渡圆直径
#endif
#ifndef HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR
#define HOME_BASE_PAGE_TRANSITION_CIRCLE_COLOR       (0x181818)
#endif
#ifndef HOME_BASE_PAGE_TRANSITION_CIRCLE_GAP
#define HOME_BASE_PAGE_TRANSITION_CIRCLE_GAP         (24)    /// 两圆边缘间距
#endif
#ifndef HOME_BASE_PAGE_TRANSITION_OPA_MIN
#define HOME_BASE_PAGE_TRANSITION_OPA_MIN            (102)   /// ~40%
#endif
#ifndef HOME_BASE_PAGE_TRANSITION_MAIN_BG_FULL_SIZE
#define HOME_BASE_PAGE_TRANSITION_MAIN_BG_FULL_SIZE  (360)   /// page0 全尺寸，过渡不缩放
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
    lv_obj_t *time_m[HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_s[HOME_BASE_MAIN_TIME_DIGIT_COUNT];
    lv_obj_t *time_colon;               /// 冒号 BAJ
    lv_obj_t *hint_text;                /// Press to start pumping
    lv_obj_t *hint_shine;               /// 扫光亮色文案（裁剪窗内）
    lv_obj_t *hint_mask;                /// 扫光裁剪窗
#if HOME_BASE_MAIN_HINT_SHINE_USE_IMG_MASK
    lv_draw_buf_t *hint_mask_a8;        /// TEXT_SHINE_MASK → A8 蒙层
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

/// 模式选择页固定层（挂 root，不参与切页位移动画）
#define HOME_BASE_MODE_PAGE_DOT_MAX                      (6)

typedef struct {
    lv_obj_t *dots[HOME_BASE_MODE_PAGE_DOT_MAX];
    lv_obj_t *tip_img;                  /// 提示文案（本模块预解码，勿释放）
    lv_obj_t *tip_shine;                /// 扫光亮色图（裁剪窗内）
    lv_obj_t *tip_mask;                 /// 扫光裁剪窗
#if HOME_BASE_MODE_TIP_SHINE_USE_IMG_MASK
    lv_draw_buf_t *tip_mask_a8;         /// TEXT_SHINE_MASK → A8 蒙层
#endif
    uint8_t tip_predecoded;
    uint8_t dot_count;
    uint8_t selected_dot;
    uint8_t ready;
    int16_t tip_mask_x;
    uint32_t tip_mask_tick_ms;
} home_base_mode_chrome_info_t;

/// 模式/韵律选择页控件（挂在 page user_data，避免切页过渡覆盖）
typedef struct {
    lv_obj_t *circle_bg;                /// 切换动画圆背景（不缩放）
    lv_obj_t *icon;
    lv_obj_t *title;
    uint8_t page_index;
    uint8_t mode;
    uint16_t icon_id;
    uint8_t icon_predecoded;
    char shown_title[24];
} home_base_mode_page_ctx_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    home_base_level_info_t level;
    home_base_mode_info_t mode;
    home_base_page_info_t page;
    home_base_animation_info_t animation;
    home_base_main_page_info_t main;
    home_base_mode_chrome_info_t mode_chrome; /// 定位点 + 提示文案（root 固定层）
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

void home_main_page_ui_build(home_base_ui_info_t *info);
void home_main_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
void home_main_page_ui_remove(home_base_ui_info_t *info);
/// 切换动画：page0 不缩放仅位移；模式页由 circle + hero 处理
void home_main_page_transition_apply(home_base_ui_info_t *info, int32_t hero_scale,
    int32_t bg_scale, bool transitioning);

void home_mode_page_ui_build(home_base_ui_info_t *info, uint8_t page_index);
void home_mode_page_ui_update(home_base_ui_info_t *info, ui_proc_home_t *proc);
void home_mode_page_ui_remove(home_base_ui_info_t *info);
/// 切换动画：圆背景显隐 + ICON/模式名缩放（圆不参与缩放）
void home_mode_page_transition_apply(lv_obj_t *page, int32_t hero_scale, bool transitioning);
/// 定位点/提示文案固定层（root）：page0 隐藏提示；离 page0 动画起显示定位点
void home_mode_chrome_prepare(home_base_ui_info_t *info);
void home_mode_chrome_remove(home_base_ui_info_t *info);
void home_mode_chrome_update(home_base_ui_info_t *info);

void home_base_animation_remove(home_base_ui_info_t *info);
void home_base_animation_pause(home_base_ui_info_t *info);
void home_base_animation_resume(home_base_ui_info_t *info);

void home_base_ui_prepare(ui_manager_context_t *thiz);
void home_base_ui_remove(ui_manager_context_t *thiz);
int home_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __HOME_BASE_UI_INTERNAL_H__
