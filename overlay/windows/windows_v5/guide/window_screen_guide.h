#ifndef __WINDOW_SCREEN_GUIDE_H__
#define __WINDOW_SCREEN_GUIDE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "windows_map_define.h" // IWYU pragma: keep
#include "input_key_code.h" // IWYU pragma: keep

#if IS_SUPPORT_LVGL
#define GUIDE_ANIMATION_FREQ                 (25)
#else
#define GUIDE_ANIMATION_FREQ                 (5)
#endif

/// 主界面动画展示时间
#define GUIDE_ANIMATION_DISPLAY_TIME            (2000) /// 2s
#define GUIDE_BLINK_PERIOD                      (500)
/// 首次进入带倒计时页的静默时间（UI 不显示倒计时）
#define GUIDE_IDLE_SILENT_FIRST_MS              (7000) /// 7s
/// 力度调节页有调节后，静默时间重置为
#define GUIDE_IDLE_SILENT_LEVEL_TOUCH_MS        (3000) /// 3s
/// 模式选择页有切换后，静默时间重置为
#define GUIDE_IDLE_SILENT_MODE_TOUCH_MS         (5000) /// 5s
/// 韵律选择页有切换后，静默时间重置为
#define GUIDE_IDLE_SILENT_RHYTHM_TOUCH_MS       (5000) /// 5s
/// 静默结束后的可见倒计时时长，结束后切下一页
#define GUIDE_IDLE_COUNTDOWN_SHOW_MS            (3000) /// 3s
/// 力度调节后，展示页面停留时间
#define GUIDE_GUIDE_STEP_ADVANCE_TIME_MS        (3000) /// 纯引导页自动翻页时间
#define GUIDE_FINISH_IDLE_TIMEOUT_MS            (3000) /// 引导结束页无操作超时

/** 静默结束后剩余可见倒计时秒数；静默中或已超时返回 0（UI 应隐藏） */
static inline uint8_t guide_idle_countdown_remain_sec(uint32_t elapsed_ms, uint32_t silent_ms)
{
    uint32_t total_ms = silent_ms + GUIDE_IDLE_COUNTDOWN_SHOW_MS;

    if (elapsed_ms < silent_ms || elapsed_ms >= total_ms) {
        return 0;
    }
    return (uint8_t)((total_ms - elapsed_ms + 999U) / 1000U);
}

static inline uint32_t guide_idle_total_timeout_ms(uint32_t silent_ms)
{
    return silent_ms + GUIDE_IDLE_COUNTDOWN_SHOW_MS;
}
#define GUIDE_PUMP_STOP_RETURN_LONG_PRESS_MS    (3000) /// 泵奶结束引导长按返回跳转时间
#define GUIDE_DEFAULT_LEFT_LEVEL                (12)
#define GUIDE_DEFAULT_RIGHT_LEVEL               (6)
#define GUIDE_LEVEL_MIN                         (0)
#define GUIDE_LEVEL_MAX                         (15)

/// 引导页模式/韵律切换是否同步到 breast_pump_manager
#ifndef IS_SUPPORT_GUIDE_PUMP_MODE_SYNC
#define IS_SUPPORT_GUIDE_PUMP_MODE_SYNC       (1)
#endif

/// 引导页力度调节是否同步到 breast_pump_manager
#ifndef IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC
#define IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC      (1)
#endif

/// 是否支持引导中启动泵阀
#ifndef IS_SUPPORT_GUIDE_PUMP_START
#define IS_SUPPORT_GUIDE_PUMP_START           (1)
#endif

/// 主界面UI动作枚举
typedef enum {
    GUIDE_HDR_ACTION_MAX
} guide_ui_hdr_action_e;

/// 主界面UI类型枚举
typedef enum {
    GUIDE_UI_TYPE_NONE = 0,
    GUIDE_UI_TYPE_HOME,
    GUIDE_UI_TYPE_MAX
} guide_ui_type_e;

typedef enum {
    GUIDE_PAGE_WELCOME_ANIM = 0,        /// 1. 引导动画
    GUIDE_PAGE_INTRO = 1,               /// 2. 引导简介
    GUIDE_PAGE_KEY_GUIDE = 2,           /// 3. 按键引导
    GUIDE_PAGE_KNOB_GUIDE = 3,          /// 4. 旋钮引导
    GUIDE_PAGE_FUNCTION_HOME = 4,       /// 5. 功能引导首页
    GUIDE_PAGE_LEVEL_BOTH_GUIDE = 5,    /// 6. 双边力度调节引导
    GUIDE_PAGE_LEVEL_BOTH = 6,          /// 7. 双边力度调节
    GUIDE_PAGE_LEVEL_LEFT_GUIDE = 7,    /// 8. 左边力度调节引导
    GUIDE_PAGE_LEVEL_LEFT = 8,          /// 9. 左边力度调节
    GUIDE_PAGE_LEVEL_RIGHT_GUIDE = 9,   /// 10. 右边力度调节引导
    GUIDE_PAGE_LEVEL_RIGHT = 10,        /// 11. 右边力度调节
    GUIDE_PAGE_MODE_SELECT_GUIDE = 11,  /// 12. 模式选择引导
    GUIDE_PAGE_MODE_SELECT = 12,        /// 13. 模式选择
    GUIDE_PAGE_RHYTHM_SELECT_GUIDE = 13,/// 14. 韵律选择引导
    GUIDE_PAGE_RHYTHM_SELECT = 14,      /// 15. 韵律选择
    GUIDE_PAGE_PUMP_PAUSE_GUIDE = 15,   /// 16. 泵奶暂停引导
    GUIDE_PAGE_PUMP_STOP_GUIDE = 16,    /// 17. 泵奶结束引导
    GUIDE_PAGE_FINISH = 17,             /// 18. 引导结束
    GUIDE_PAGE_MAX,
} guide_page_index_t;

typedef enum {
    GUIDE_MODE_SELECT_NONE = 0, /// 未选择模式
    GUIDE_MODE_SELECT_MODE = 1, /// 选择模式
    GUIDE_MODE_SELECT_BREASTFEEDING = 2, /// 选择哺乳
    GUIDE_MODE_SELECT_MAX,
} guide_mode_select_e;

#define GUIDE_PAGE_IS_WELCOME_ANIM(page)        ((page) == GUIDE_PAGE_WELCOME_ANIM)
#define GUIDE_PAGE_IS_LEVEL_INTERACTIVE(page)   \
    ((page) == GUIDE_PAGE_LEVEL_BOTH || (page) == GUIDE_PAGE_LEVEL_LEFT || \
     (page) == GUIDE_PAGE_LEVEL_RIGHT)
#define GUIDE_PAGE_IS_MODE_INTERACTIVE(page)    ((page) == GUIDE_PAGE_MODE_SELECT)
#define GUIDE_PAGE_IS_RHYTHM_INTERACTIVE(page)  ((page) == GUIDE_PAGE_RHYTHM_SELECT)
#define GUIDE_PAGE_IS_LEVEL_SIDE(page)        \
    ((page) == GUIDE_PAGE_LEVEL_LEFT || (page) == GUIDE_PAGE_LEVEL_RIGHT)
#define GUIDE_PAGE_IS_GUIDE_STEP(page)        \
    ((page) == GUIDE_PAGE_INTRO || (page) == GUIDE_PAGE_KEY_GUIDE || \
     (page) == GUIDE_PAGE_KNOB_GUIDE || (page) == GUIDE_PAGE_LEVEL_BOTH_GUIDE || \
     (page) == GUIDE_PAGE_LEVEL_LEFT_GUIDE || (page) == GUIDE_PAGE_LEVEL_RIGHT_GUIDE || \
     (page) == GUIDE_PAGE_MODE_SELECT_GUIDE || (page) == GUIDE_PAGE_RHYTHM_SELECT_GUIDE || \
     (page) == GUIDE_PAGE_PUMP_PAUSE_GUIDE || (page) == GUIDE_PAGE_PUMP_STOP_GUIDE)
#define GUIDE_PAGE_SHOW_BLE(page)               \
    ((page) == GUIDE_PAGE_MODE_SELECT || (page) == GUIDE_PAGE_RHYTHM_SELECT)
#define GUIDE_PAGE_SHOW_LEVEL(page)             \
    (((page) >= GUIDE_PAGE_LEVEL_BOTH_GUIDE && (page) <= GUIDE_PAGE_LEVEL_RIGHT) || \
     (page) == GUIDE_PAGE_MODE_SELECT_GUIDE || (page) == GUIDE_PAGE_MODE_SELECT || \
     (page) == GUIDE_PAGE_RHYTHM_SELECT_GUIDE || (page) == GUIDE_PAGE_RHYTHM_SELECT)
#define GUIDE_PAGE_IS_LED_BLINK(page)           \
    ((page) == GUIDE_PAGE_INTRO || \
     (page) == GUIDE_PAGE_FUNCTION_HOME || \
     (page) == GUIDE_PAGE_LEVEL_LEFT_GUIDE || (page) == GUIDE_PAGE_LEVEL_RIGHT_GUIDE || \
     (page) == GUIDE_PAGE_MODE_SELECT_GUIDE || (page) == GUIDE_PAGE_MODE_SELECT || \
     (page) == GUIDE_PAGE_RHYTHM_SELECT_GUIDE || (page) == GUIDE_PAGE_RHYTHM_SELECT || \
     (page) == GUIDE_PAGE_PUMP_PAUSE_GUIDE || (page) == GUIDE_PAGE_PUMP_STOP_GUIDE)

/// 主界面UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    ble_manager_context_t *ble;
    battery_manager_context_t *battery;
    led_manager_context_t *led;
    key_input_context_t *key;
    breast_pump_manager_context_t *pump;
    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. guide_ui_type_e
    uint16_t page_index; /// 当前页面索引.
    uint16_t popup_ui_type; /// 当前弹窗类型. guide_operation_type_e
    pump_setting_t setting; /// 泵奶设置信息.
    uint8_t mode_selected; /// 是否已选择吸乳方案
    uint8_t ui_mode_index; /// 当前模式索引 breast_pump_mode_e
    uint8_t state_blink_visible; /// 状态闪烁可见: 0: 隐藏相, 1: 显示相.
    uint8_t level_knob_touched; /// 力度页是否已旋转过旋钮.
    uint8_t mode_select_touched; /// 模式选择页是否已切换过模式.
    uint8_t rhythm_select_touched; /// 韵律选择页是否已切换过韵律.
    uint8_t exit_holding; /// 退出倒计时弹窗是否处于长按中
    uint8_t exit_ignore_click; /// 退出倒计时松手后忽略紧随的 CLICK
    uint32_t last_adjust_time; /// 上次力度调节时间
    uint32_t page_enter_time; /// 当前页面进入时间
    uint32_t blink_start_time; /// 闪烁起始时间
} ui_proc_guide_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_guide_ui_animation(ui_proc_guide_t *proc);
void stop_guide_ui_animation(ui_proc_guide_t *proc);
void guide_led_backlight_update(ui_proc_guide_t *proc);
void guide_led_backlight_blink_tick(ui_proc_guide_t *proc);
void guide_page_advance(ui_proc_guide_t *proc, uint16_t next_page);
int guide_pump_mode_adjust_handler(ui_proc_guide_t *proc, bool is_bf_mode);
void guide_pump_level_page_enter_reset(ui_proc_guide_t *proc);
int guide_pump_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase);
void guide_pump_level_idle_timeout_handler(ui_proc_guide_t *proc);
uint8_t guide_pump_level_idle_remain_sec_get(const ui_proc_guide_t *proc);
uint8_t guide_mode_select_idle_remain_sec_get(const ui_proc_guide_t *proc);
uint8_t guide_rhythm_select_idle_remain_sec_get(const ui_proc_guide_t *proc);
void guide_knob_event_handler(ui_proc_guide_t *proc, bool is_left);
void guide_key_event_handler(ui_proc_guide_t *proc, uint16_t keycode, uint16_t keytype);
void guide_exit_countdown_popup_dismiss(ui_proc_guide_t *proc);
void guide_cancel_key_down_handler(ui_proc_guide_t *proc);
void guide_cancel_key_up_handler(ui_proc_guide_t *proc);
#ifdef __cplusplus
}
#endif

#endif 
