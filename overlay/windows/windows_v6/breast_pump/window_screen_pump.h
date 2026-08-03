#ifndef __WINDOW_SCREEN_PUMP_H__
#define __WINDOW_SCREEN_PUMP_H__

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

/// 泵奶动画频率
#define PUMP_ANIMATION_FREQ                    (20)
#define PUMP_SPEED_ANIMATION_FREQ              (25)
#define PUMP_LEVEL_ANIMATION_FREQ              (20) 

/// 是否支持泵奶中切换韵律
#define IS_SUPPORT_PUMP_IN_MODE_SWITCH_RHYTHM  (1)

/**
 * 切换韵律确认页策略（二选一，默认方案1）：
 * 方案1：目标为韵律（模式→韵律 / 韵律→韵律），且本次泵奶首次才弹确认页
 * 方案2：目标为韵律模式即弹确认页（每次切换都弹）
 */
#define PUMP_RHYTHM_CONFIRM_SCHEME_1           (1)
#define PUMP_RHYTHM_CONFIRM_SCHEME_2           (2)
#ifndef PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME
#define PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME      PUMP_RHYTHM_CONFIRM_SCHEME_1
#endif

/// 泵奶主界面档位调节展示时间
#define PUMP_LEVEL_DISPLAY_TIMEOUT             (3000) /// 3s
/// 单边力度连续加档限制窗口 / 过大提示时长
#define PUMP_LEVEL_OVER_LIMIT_WINDOW_MS        (2000) /// 2s
#define PUMP_LEVEL_OVER_LIMIT_MAX_STEP         (3)    /// 窗口内最多加 3 档
/// 子模式切换提醒时长
#define PUMP_SUB_MODE_SWITCH_TIMEOUT_MS        (3000) /// 3s

/// 模式选择页无操作静默回首页（page_index 非 0 时计时，超时仅复位预览；
/// 不更换工作模式，模式落地仅由 power 确认触发）
#ifndef PUMP_MODE_SELECT_IDLE_EXIT_ENABLE
#define PUMP_MODE_SELECT_IDLE_EXIT_ENABLE      (1)
#endif
#define PUMP_MODE_SELECT_IDLE_TIMEOUT_MS       (5000) /// 5s

/// 力度/速度调节页退出：1=回到上一页（保留 page_index），0=回本窗口首页（page_index 清零）
#ifndef PUMP_ADJUST_EXIT_TO_PREV_PAGE_ENABLE
#define PUMP_ADJUST_EXIT_TO_PREV_PAGE_ENABLE   (0)
#endif

/// 子模式同步场景（力度/速度页共用）
typedef enum {
    PUMP_SUB_MODE_SYNC_LEVEL = 0,      /// 单边力度
    PUMP_SUB_MODE_SYNC_BOTH_LEVEL,     /// 双边力度
    PUMP_SUB_MODE_SYNC_SPEED,          /// 速度
} pump_sub_mode_sync_e;
/// 模式切换成功页展示时长
#define PUMP_SETTINGS_SUCCESS_DISPLAY_TIMEOUT  (1500) /// 1.5s
/// 泵奶背光灯周期（半周期 500ms → 全周期 1Hz）
#define PUMP_BLINK_PERIOD                      (500)

/// 泵奶按键灯状态
typedef enum {
    PUMP_LIGHT_STATUS_DEFAULT = 0,      /// 默认常亮
    PUMP_LIGHT_STATUS_PAUSE,            /// 暂停：start 灯闪烁
    PUMP_LIGHT_STATUS_MAX
} pump_light_status_e;

/// 泵奶UI动作枚举
typedef enum {
    PUMP_HDR_ACTION_START_PUMP = 100,
    PUMP_HDR_ACTION_PUMP_COMPLETE,
    PUMP_HDR_ACTION_MAX
} pump_ui_hdr_action_e;

/// 泵奶UI类型枚举
typedef enum {
    PUMP_UI_TYPE_NONE = 0,
    PUMP_UI_TYPE_HOME = 100,
    PUMP_UI_TYPE_RHYTHM_CONFIRM, /// 切换韵律确认
    PUMP_UI_TYPE_SETTINGS_SUCCESS, /// 模式切换成功
    PUMP_UI_TYPE_STOP_HOLD, /// 长按 power 停止泵奶确认
    PUMP_UI_TYPE_LEVEL_ADJUST, /// 单边力度调节
    PUMP_UI_TYPE_BOTH_LEVEL_ADJUST, /// 双边力度调节
    PUMP_UI_TYPE_SPEED_ADJUST, /// 速度调节
    PUMP_UI_TYPE_FINISH_TIMEOUT, /// 泵奶超时结算
    PUMP_UI_TYPE_WILL_FINISH, /// 韵律即将结束提示
    PUMP_UI_TYPE_MAX
} pump_ui_type_e;

typedef enum {
    PUMP_LEVEL_ADJUST_SIDE_LEFT = 0,
    PUMP_LEVEL_ADJUST_SIDE_RIGHT = 1,
} pump_level_adjust_side_e;

/// 与 PUMP_HOME_PAGE_* 对齐，用于驱动模式选择页切换
typedef enum {
    PUMP_MODE_PAGE_INDEX_NONE = 0,                   /// 主页（无模式选择）
    PUMP_MODE_PAGE_INDEX_STIMULATION_SELECT = 1,     /// 刺激模式选择
    PUMP_MODE_PAGE_INDEX_EXPRESSION_SELECT = 2,      /// 吸乳模式选择
    PUMP_MODE_PAGE_INDEX_MILK_BOOST_SELECT = 3,      /// 奶量提升选择
    PUMP_MODE_PAGE_INDEX_POWER_PUMPING_SELECT = 4,   /// Power Pumping选择
    PUMP_MODE_PAGE_INDEX_CUSTOMIZE_SELECT = 5,       /// 个性化选择
} pump_mode_page_index_e;

/**
 * 子页返回策略（show 时 push，dismiss 时选策略）
 * - HISTORY：弹出栈顶，恢复进入前 UI（力度/速度/首页+page_index）
 * - HOME_KEEP_PAGE：回窗口首页，保留/恢复 page_index（模式选择页）
 * - HOME_RESET_PAGE：回窗口首页，page_index 清零
 */
typedef enum {
    PUMP_UI_NAV_BACK_HISTORY = 0,
    PUMP_UI_NAV_BACK_HOME_KEEP_PAGE,
    PUMP_UI_NAV_BACK_HOME_RESET_PAGE,
} pump_ui_nav_back_e;

#define PUMP_UI_NAV_DEPTH_MAX                       (2)

typedef struct {
    uint16_t ui_type;       /// pump_ui_type_e
    uint16_t page_index;    /// ui_mode_page_index
    uint8_t level_side;     /// 单边力度侧（仅 LEVEL_ADJUST 有意义）
} pump_ui_nav_frame_t;

typedef struct {
    pump_ui_nav_frame_t stack[PUMP_UI_NAV_DEPTH_MAX];
    uint8_t depth;
    uint8_t restoring;      /// nav_back 恢复中，避免 show 再次 push
} pump_ui_nav_t;

/// 泵奶UI结构体
typedef struct {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    breast_pump_manager_context_t *pump;
    battery_manager_context_t *battery;
    ble_manager_context_t *ble;
    motor_manager_context_t *motor;
    power_manager_context_t *pm;
    key_input_context_t *key;
    algo_manager_context_t *algo;
    led_manager_context_t *led;
    airplane_mode_context_t *airplane_mode;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic;
    uint8_t window_run_state; /// WM是否运行.
    uint16_t current_ui_type; /// 当前UI类型
    uint16_t popup_ui_type; /// 当前弹窗类型. popup_ui_id_e
    uint32_t last_progress_ani_time; /// 上次进度动画时间

    /// 泵奶信息
    breast_pump_result_t pump_result; /// 泵奶结果.
    uint8_t pump_status; /// 泵奶状态.

    /// 模式相关
    uint16_t ui_mode_page_index; /// 当前UI模式页面索引（与 home 一致，驱动切页）
    uint32_t mode_select_idle_time; /// 模式选择页无操作计时起点
    uint8_t rhythm_confirm_shown; /// 本次泵奶是否已展示过切换韵律确认页

    /// 长按停止确认
    uint8_t stop_hold_holding; /// 停止确认页是否处于按住倒计时
    uint8_t stop_hold_ignore_click; /// 松手后忽略紧随的 power CLICK

    /// 子页导航栈（回历史页 / 回首页）
    pump_ui_nav_t ui_nav;

    /// 档位调节相关
    pump_setting_t setting; /// 当前泵奶设置信息.
    uint8_t level_adjust_side; /// 单边调节侧 pump_level_adjust_side_e
    uint8_t level_value; /// 实际/展示力度值（过大提示时不变）
    uint8_t level_arc_level; /// 圆弧动画目标力度（可与 level_value 分离）
    uint8_t level_over_limit; /// 力度过大提示中
    uint8_t level_inc_window_base; /// 连续加档窗口起始力度
    uint8_t sub_mode_type; /// 当前子模式 pump_mode_type_e（力度/速度页共用）
    uint8_t sub_mode_switch; /// 子模式切换提醒中（力度/速度页共用）
    int sub_mode_time_index; /// 调节页缓存的 mode step index，读写 setting 用
    uint32_t level_adjust_time; /// 档位调节页最近操作时间
    uint32_t level_over_limit_time; /// 过大提示计时起点（有调节刷新）
    uint32_t level_inc_window_time; /// 连续加档窗口起点
    uint32_t sub_mode_switch_time; /// 子模式切换提醒计时（力度/速度页共用）

    /// 双边力度调节（无超时退出，短按 power 回泵奶首页）
    uint8_t both_level_value[2]; /// 双边实际/展示力度（过大提示时不变）
    uint8_t both_level_arc[2]; /// 双边圆弧动画目标力度
    uint8_t both_level_over_limit; /// 双边力度过大提示中
    uint8_t both_level_inc_steps; /// 双边连续加档窗口内已加档次数
    uint32_t both_level_over_limit_time; /// 双边过大提示计时
    uint32_t both_level_inc_window_time; /// 双边连续加档窗口起点

    /// 灯光相关（仅按键灯，不同步 UI 组件闪烁）
    uint8_t pump_light_status; /// 泵奶灯光状态 pump_light_status_e
    uint8_t state_blink_visible; /// 闪烁相位: 0 灭, 1 亮
    uint8_t pump_pause_blink_state; /// 暂停态 start 灯上次闪烁相位
    uint32_t blink_start_time; /// 闪烁起始时间

} ui_proc_pump_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 显示泵奶主界面
void show_pump_home_ui(ui_proc_pump_t *proc);
/// 泵奶状态UI相关
void switch_pump_status_ui(ui_proc_pump_t *proc);
/// 子页导航：进入子页前 push；退出时按策略 back
void pump_ui_nav_push(ui_proc_pump_t *proc);
void pump_ui_nav_clear(ui_proc_pump_t *proc);
void pump_ui_nav_back(ui_proc_pump_t *proc, pump_ui_nav_back_e policy);
/// 移除UI相关
void pump_ui_change_hdr_remove(ui_proc_pump_t *proc);
/// 停止泵奶UI动画
void stop_pump_ui_animation(ui_proc_pump_t *proc);
/// 泵奶完成事件处理
void pump_complete_event_handler(ui_proc_pump_t *proc);
/// 弹窗类型更新
void pump_popup_ui_type_update(ui_proc_pump_t *proc, popup_ui_id_e type);
/// 暂停态 start 按键灯 1Hz 闪烁 tick（仅 LED，不同步 UI）
void pump_pause_led_tick(ui_proc_pump_t *proc);

/// 子模式（刺激/吸乳/混合/放松）同步：力度/速度页共用
uint8_t pump_sub_mode_type_get(ui_proc_pump_t *proc);
void pump_sub_mode_switch_clear(ui_proc_pump_t *proc);
void pump_sub_mode_switch_tick(ui_proc_pump_t *proc);
void pump_sub_mode_state_init(ui_proc_pump_t *proc, uint8_t clamp_level);
void pump_sub_mode_sync_tick(ui_proc_pump_t *proc, pump_sub_mode_sync_e sync);

/// 电源按键事件处理
void pump_power_key_event_handler(ui_proc_pump_t *proc);
/// 开始按键事件处理
void pump_start_key_event_handler(ui_proc_pump_t *proc);

/// 模式选择：旋钮预览 / 确认落地 / 直接设模式
int pump_mode_adjust_handler(ui_proc_pump_t *proc, bool is_left);
int pump_mode_confirm_handler(ui_proc_pump_t *proc);
int pump_mode_set_handler(ui_proc_pump_t *proc, uint8_t mode_index);
void pump_mode_page_index_clamp(ui_proc_pump_t *proc);
/// 模式选择页无操作自动回首页：按键刷新计时 / 超时检测
void pump_mode_select_idle_touch(ui_proc_pump_t *proc);
void pump_mode_select_idle_timeout_handler(ui_proc_pump_t *proc);
/// 是否需要先弹切换韵律确认页（0=需要，负数=不需要/错误）
int pump_mode_rhythm_confirm_needed(ui_proc_pump_t *proc);

/// 切换韵律确认页
void show_pump_rhythm_confirm_ui(ui_proc_pump_t *proc);
void pump_rhythm_confirm_ui_exit(ui_proc_pump_t *proc);

/// 模式切换成功页
void show_pump_settings_success_ui(ui_proc_pump_t *proc);
void pump_settings_success_ui_exit(ui_proc_pump_t *proc);

/// 长按 power 停止泵奶确认页
void show_pump_stop_hold_ui(ui_proc_pump_t *proc);
void pump_stop_hold_ui_exit(ui_proc_pump_t *proc);
void pump_stop_hold_dismiss(ui_proc_pump_t *proc);
void pump_stop_hold_key_long_2s_handler(ui_proc_pump_t *proc);
void pump_stop_hold_key_up_handler(ui_proc_pump_t *proc);

/// 单边力度调节
void show_pump_level_adjust_ui(ui_proc_pump_t *proc, uint8_t is_left);
int pump_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase);
void pump_level_adjust_ui_exit(ui_proc_pump_t *proc);
void pump_level_adjust_ui_dismiss(ui_proc_pump_t *proc);

/// 双边力度调节（同时短按 L+R 进入，无超时退出，短按 power 回泵奶首页）
void show_pump_both_level_adjust_ui(ui_proc_pump_t *proc);
int pump_both_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase);
void pump_both_level_adjust_ui_exit(ui_proc_pump_t *proc);
void pump_both_level_adjust_ui_dismiss(ui_proc_pump_t *proc);

/// 速度调节（无超时退出，短按 power 回泵奶首页）
void show_pump_speed_adjust_ui(ui_proc_pump_t *proc);
int pump_speed_adjust_handler(ui_proc_pump_t *proc, bool is_increase);
void pump_speed_adjust_ui_exit(ui_proc_pump_t *proc);
void pump_speed_adjust_ui_dismiss(ui_proc_pump_t *proc);

/// 泵奶超时结算页
void show_pump_finish_timeout_ui(ui_proc_pump_t *proc);
void pump_finish_timeout_ui_exit(ui_proc_pump_t *proc);

/// 韵律即将结束提示页（短按 power 切吸吮并回首页；5s 超时仅回首页不切模式；其它按键不响应）
void show_pump_will_finish_ui(ui_proc_pump_t *proc);
void pump_will_finish_ui_exit(ui_proc_pump_t *proc);
void pump_will_finish_ui_dismiss(ui_proc_pump_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


