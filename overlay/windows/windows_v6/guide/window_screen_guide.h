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

#define GUIDE_BLINK_PERIOD                      (500)
/// 左/右力度页：最后一次旋转后静默结算时长，结算完成后等待再跳转
#define GUIDE_LEVEL_KNOB_ADVANCE_MS             (2000) /// 2s
/// 左/右力度页：结算完成后的跳转等待时长（UI 不展示）
#define GUIDE_LEVEL_SIDE_COUNTDOWN_MS           (1000) /// 1s
/// 双边力度调节-加页：右侧档位 >= 目标档位后开始结算
#define GUIDE_LEVEL_BOTH_INCREASE_TARGET        (BREAST_PUMP_LEVEL_5)
/// 双边力度调节-减页：左侧档位 <= 目标档位后开始结算
#define GUIDE_LEVEL_BOTH_DECREASE_TARGET        (BREAST_PUMP_LEVEL_1)
/// 双边力度调节-加/减页：到达目标档位后的结算时长，结算完成后跳转
#define GUIDE_LEVEL_BOTH_SETTLE_MS              (1000) /// 1s
/// 力度调节过大限制与提示（单边/双边共用，默认打开）
#ifndef GUIDE_LEVEL_OVER_LIMIT_ENABLE
#define GUIDE_LEVEL_OVER_LIMIT_ENABLE           (1)
#endif
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
#define GUIDE_LEVEL_OVER_LIMIT_WINDOW_MS        (2000) /// 2s
/// 调节过大后静默，进度条开始回退
#define GUIDE_LEVEL_OVER_LIMIT_ARC_ROLLBACK_MS  (1000) /// 1s
/// 调节过大后静默，过大提醒文案消失
#define GUIDE_LEVEL_OVER_LIMIT_TIP_CLEAR_MS     (2000) /// 2s
#define GUIDE_LEVEL_OVER_LIMIT_MAX_STEP         (3)    /// 窗口内最多加 3 档
#endif
/// 力度调节后，展示页面停留时间
#define GUIDE_GUIDE_STEP_ADVANCE_TIME_MS        (3000) /// 纯引导页自动翻页时间
#define GUIDE_PUMP_HOME_GUIDE_ADVANCE_TIME_MS   (3200) /// 第四页泵奶首页引导停留后切第五页
#define GUIDE_FINISH_IDLE_TIMEOUT_MS            (3000) /// 引导结束页无操作超时

/// 引导页模式/韵律切换是否同步到 breast_pump_manager
#ifndef IS_SUPPORT_GUIDE_PUMP_MODE_SYNC
#define IS_SUPPORT_GUIDE_PUMP_MODE_SYNC          (0)
#endif

/// 引导页力度调节是否同步到 breast_pump_manager
#ifndef IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC
#define IS_SUPPORT_GUIDE_PUMP_LEVEL_SYNC         (0)
#endif

/// 是否支持引导中启动泵阀
#ifndef IS_SUPPORT_GUIDE_PUMP_START
#define IS_SUPPORT_GUIDE_PUMP_START              (0)
#endif

/// 引导已部署标记设置时机：1=进入引导页时，0=退出引导页时
#ifndef GUIDE_SET_DEPLOYED_FLAG_ON_ENTER
#define GUIDE_SET_DEPLOYED_FLAG_ON_ENTER           (1)
#endif

/// 主界面UI动作枚举
typedef enum {
    GUIDE_HDR_ACTION_MAX
} guide_ui_hdr_action_e;

/// 主界面UI类型枚举
typedef enum {
    GUIDE_UI_TYPE_NONE = 0,
    GUIDE_UI_TYPE_HOME,
    GUIDE_UI_TYPE_POWER_OFF_HOLD, /// 长按 power 关机确认
    GUIDE_UI_TYPE_MAX
} guide_ui_type_e;

typedef enum {
    GUIDE_PAGE_QRCODE = 0,              /// 1. 绑定二维码
    GUIDE_PAGE_INTRO = 1,               /// 2. 引导简介
    GUIDE_PAGE_PUMP_START_GUIDE = 2,    /// 3. 泵奶启动向导
    GUIDE_PAGE_PUMP_HOME_GUIDE = 3,     /// 4. 泵奶首页引导（刺激模式）
    GUIDE_PAGE_PUMP_PAUSE_GUIDE = 4,    /// 5. 泵奶暂停向导
    GUIDE_PAGE_PUMP_PAUSE = 5,          /// 6. 泵奶暂停（刺激模式）
    GUIDE_PAGE_PUMP_FINISH_GUIDE = 6,   /// 7. 泵奶完成向导
    GUIDE_PAGE_PUMP_FINISH_HOLD = 7,    /// 8. 泵奶完成进度页
    GUIDE_PAGE_PUMP_HOME = 8,           /// 9. 泵奶首页
    GUIDE_PAGE_MODE_SWITCH_GUIDE = 9,   /// 10. 模式切换向导
    GUIDE_PAGE_MODE_SWITCH = 10,        /// 11. 模式切换
    GUIDE_PAGE_MODE_HOME = 11,          /// 12. 模式首页
    GUIDE_PAGE_LEVEL_SIDE_GUIDE = 12,   /// 13. 单边力度调节向导
    GUIDE_PAGE_LEVEL_SIDE = 13,         /// 14. 单边力度调节
    GUIDE_PAGE_LEVEL_SIDE_DONE = 14,    /// 15. 单边力度调节完成
    GUIDE_PAGE_LEVEL_BOTH_GUIDE = 15,   /// 16. 双边力度调节向导
    GUIDE_PAGE_LEVEL_BOTH = 16,         /// 17. 双边力度调节
    GUIDE_PAGE_LEVEL_BOTH_DONE = 17,    /// 18. 双边力度调节完成
    GUIDE_PAGE_SPEED_GUIDE = 18,        /// 19. 速度调节向导
    GUIDE_PAGE_SPEED = 19,              /// 20. 速度调节
    GUIDE_PAGE_SPEED_DONE = 20,         /// 21. 速度调节完成
    GUIDE_PAGE_FINISH = 21,             /// 22. 引导完成
    GUIDE_PAGE_MAX,
} guide_page_index_t;

typedef enum {
    GUIDE_LEVEL_ADJUST_SIDE_LEFT = 0,
    GUIDE_LEVEL_ADJUST_SIDE_RIGHT = 1,
} guide_level_adjust_side_e;

/** 第十一页固定模式槽位（无首页、无 Custom） */
#define GUIDE_MODE_SWITCH_SLOT_STIMULATION          (0)
#define GUIDE_MODE_SWITCH_SLOT_EXPRESSION           (1)
#define GUIDE_MODE_SWITCH_SLOT_MILK_BOOST           (2)
#define GUIDE_MODE_SWITCH_SLOT_POWER_PUMPING        (3)
#define GUIDE_MODE_SWITCH_SLOT_COUNT                (4)

static inline uint8_t guide_mode_switch_mode_from_slot(uint8_t slot)
{
    switch (slot) {
        case GUIDE_MODE_SWITCH_SLOT_EXPRESSION:
            return BREAST_PUMP_LACTATION_MODE;
        case GUIDE_MODE_SWITCH_SLOT_MILK_BOOST:
            return BREAST_PUMP_MILK_BOOST_MODE;
        case GUIDE_MODE_SWITCH_SLOT_POWER_PUMPING:
            return BREAST_PUMP_PP_MILK_TRACK_MODE;
        case GUIDE_MODE_SWITCH_SLOT_STIMULATION:
        default:
            return BREAST_PUMP_STIMULATION_MODE;
    }
}

/** 兼容旧命名：旋钮按压引导即泵奶启动向导 */
#define GUIDE_PAGE_KNOB_GUIDE                       GUIDE_PAGE_PUMP_START_GUIDE

#define GUIDE_PAGE_IS_GUIDE_STEP(page)        \
    ((page) == GUIDE_PAGE_INTRO || (page) == GUIDE_PAGE_PUMP_START_GUIDE || \
     (page) == GUIDE_PAGE_PUMP_PAUSE_GUIDE || (page) == GUIDE_PAGE_PUMP_FINISH_GUIDE || \
     (page) == GUIDE_PAGE_MODE_SWITCH_GUIDE || (page) == GUIDE_PAGE_LEVEL_SIDE_GUIDE || \
     (page) == GUIDE_PAGE_LEVEL_BOTH_GUIDE || (page) == GUIDE_PAGE_SPEED_GUIDE)
#define GUIDE_PAGE_IS_LED_BLINK(page)           \
    ((page) == GUIDE_PAGE_LEVEL_SIDE_GUIDE || (page) == GUIDE_PAGE_LEVEL_BOTH_GUIDE || \
     (page) == GUIDE_PAGE_SPEED_GUIDE)

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
    uint8_t ui_mode_index; /// 模式切换页槽位 0~3（GUIDE_MODE_SWITCH_SLOT_*）
    uint8_t state_blink_visible; /// 状态闪烁可见: 0: 隐藏相, 1: 显示相.
    uint8_t level_knob_touched; /// 力度页是否已旋转过旋钮.
    uint8_t level_adjust_side; /// 单边力度调节侧 guide_level_adjust_side_e
    uint8_t level_value; /// 单边力度展示值（过大提示时不变）
    uint8_t level_arc_level; /// 单边力度圆弧目标值（可与 level_value 分离）
    uint8_t level_over_limit; /// 单边力度过大提示中
    uint8_t level_inc_window_base; /// 单边连续加档窗口起始力度
    uint32_t level_inc_window_time; /// 单边加档窗口起始时间
    uint32_t level_over_limit_time; /// 单边过大提示计时起点（有调节刷新）
    uint8_t both_level_value[2]; /// 双边实际/展示力度（过大提示时不变）
    uint8_t both_level_arc[2]; /// 双边圆弧目标力度
    uint8_t both_level_over_limit; /// 双边过大提示
    uint8_t both_level_inc_steps; /// 双边加档窗口内步数
    uint32_t both_level_inc_window_time; /// 双边加档窗口起始时间
    uint32_t both_level_over_limit_time; /// 双边过大提示起始时间
    uint8_t mode_select_touched; /// 模式选择页是否已切换过模式.
    uint8_t mode_select_click_cnt; /// 模式选择页 mode 键累计点击次数.
    uint8_t rhythm_select_touched; /// 韵律选择页是否已切换过韵律.
    uint8_t rhythm_select_click_cnt; /// 韵律选择页 program 键累计点击次数.
    uint8_t stop_hold_holding; /// 泵奶完成进度页是否处于长按中
    uint8_t power_off_hold_holding; /// 关机确认页是否处于按住倒计时
    uint8_t qrcode_only_session; /// 进入前已部署：本会话仅展示二维码
    uint32_t last_adjust_time; /// 上次力度调节时间
    uint32_t level_knob_ready_time; /// 左/右力度页：非 0 表示倒计时已锁定；双边力度页：到达目标档位后的计时起点
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
void guide_pump_level_page_enter_sync(ui_proc_guide_t *proc);
int guide_pump_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase);
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
void guide_pump_level_over_limit_tick(ui_proc_guide_t *proc);
#endif
void guide_pump_level_idle_timeout_handler(ui_proc_guide_t *proc);
uint8_t guide_pump_level_idle_remain_sec_get(const ui_proc_guide_t *proc);
void guide_pump_both_level_page_enter_sync(ui_proc_guide_t *proc);
int guide_pump_both_level_adjust_handler(ui_proc_guide_t *proc, bool is_increase);
#if GUIDE_LEVEL_OVER_LIMIT_ENABLE
void guide_pump_both_level_over_limit_tick(ui_proc_guide_t *proc);
#endif
void guide_pump_speed_page_enter_sync(ui_proc_guide_t *proc);
int guide_pump_speed_adjust_handler(ui_proc_guide_t *proc, bool is_increase);
int guide_mode_switch_adjust_handler(ui_proc_guide_t *proc, bool is_left);
void guide_knob_event_handler(ui_proc_guide_t *proc, bool is_left);
void guide_key_event_handler(ui_proc_guide_t *proc, uint16_t keycode, uint16_t keytype);
void guide_exit_to_home(ui_proc_guide_t *proc);
void guide_stop_hold_key_long_2s_handler(ui_proc_guide_t *proc);
void guide_stop_hold_key_up_handler(ui_proc_guide_t *proc);
/// 长按 power 关机确认页（第 7/8 页除外，与泵奶完成长按冲突）
void show_guide_power_off_hold_ui(ui_proc_guide_t *proc);
void guide_power_off_hold_ui_exit(ui_proc_guide_t *proc);
void guide_power_off_hold_dismiss(ui_proc_guide_t *proc);
void guide_power_off_hold_key_long_2s_handler(ui_proc_guide_t *proc);
void guide_power_off_hold_key_up_handler(ui_proc_guide_t *proc);
#ifdef __cplusplus
}
#endif

#endif 
