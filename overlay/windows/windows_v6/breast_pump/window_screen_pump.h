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
#define PUMP_LOW_POWER_COMPLETE_TASK_NAME      "pump_low_power_complete"
#define PUMP_SEAL_LEAK_TASK_NAME               "pump_seal_leak"
#define PUMP_SEAL_RESULT_TIP_TASK_NAME         "pump_seal_result_tip" 

/// 是否支持泵奶中切换韵律
#define IS_SUPPORT_PUMP_IN_MODE_SWITCH_RHYTHM  (1)

/**
 * 切换韵律确认页策略（二选一，默认方案1）：
 * 方案1：目标为韵律（模式→韵律 / 韵律→韵律），且上下电周期内首次才弹确认页
 * 方案2：目标为韵律模式即弹确认页（每次切换都弹）
 */
#define PUMP_RHYTHM_CONFIRM_SCHEME_1           (1)
#define PUMP_RHYTHM_CONFIRM_SCHEME_2           (2)
#ifndef PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME
#define PUMP_RHYTHM_SWITCH_CONFIRM_SCHEME      PUMP_RHYTHM_CONFIRM_SCHEME_1
#endif

/// 单边/双边力度连续加档限制窗口时长
#define PUMP_LEVEL_OVER_LIMIT_WINDOW_MS        (2000) /// 2s
/// 调节过大后静默，进度条开始回退
#define PUMP_LEVEL_OVER_LIMIT_ARC_ROLLBACK_MS  (1000) /// 1s
/// 调节过大后静默，过大提醒文案消失
#define PUMP_LEVEL_OVER_LIMIT_TIP_CLEAR_MS     (2000) /// 2s
#define PUMP_LEVEL_OVER_LIMIT_MAX_STEP         (3)    /// 窗口内最多加 3 档
/// 子模式切换提醒时长
#define PUMP_SUB_MODE_SWITCH_TIMEOUT_MS        (3000) /// 3s

/// 漏气检测：通过文案展示时长
#define PUMP_SEAL_RESULT_TIMEOUT_MS            (3000)  /// 3s

/// 模式选择页无操作静默回首页（page_index 非 0 时计时，超时仅复位预览；
/// 不更换工作模式，模式落地仅由 power 确认触发）
#ifndef PUMP_MODE_SELECT_IDLE_EXIT_ENABLE
#define PUMP_MODE_SELECT_IDLE_EXIT_ENABLE      (1)
#endif
#define PUMP_MODE_SELECT_IDLE_TIMEOUT_MS       (5000) /// 5s

/// 切换韵律确认页：无操作静默返回（不落地模式）
#ifndef PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE
#define PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE   (1)
#endif
#if PUMP_RHYTHM_CONFIRM_IDLE_EXIT_ENABLE
#define PUMP_RHYTHM_CONFIRM_IDLE_TIMEOUT_MS    (5000) /// 5s
#endif

/// 切换韵律确认页：非短按 power 退出记入无效（默认关闭）
/// 开启后：仅短按 power 确认才计为已提示；短按其它按键/静默退出不消耗首次提示次数
#ifndef PUMP_RHYTHM_CONFIRM_INVALID_EXIT_ENABLE
#define PUMP_RHYTHM_CONFIRM_INVALID_EXIT_ENABLE (1)
#endif

/// 力度调节页：非力度限制态无操作静默退出
#define PUMP_LEVEL_ADJUST_IDLE_TIMEOUT_MS      (5000) /// 5s
/// 速度调节页：无操作静默退出
#define PUMP_SPEED_ADJUST_IDLE_TIMEOUT_MS      (5000) /// 5s

/// 力度/速度调节页：进入后再次点击对应按键退出的防抖保护
#ifndef PUMP_ADJUST_KEY_EXIT_PROTECT_ENABLE
#define PUMP_ADJUST_KEY_EXIT_PROTECT_ENABLE    (1)
#endif
#define PUMP_ADJUST_KEY_EXIT_PROTECT_MS        (100) /// 100ms


/// 力度/速度调节页退出：1=回到上一页（保留 page_index），0=回本窗口首页（page_index 清零）
#ifndef PUMP_ADJUST_EXIT_TO_PREV_PAGE_ENABLE
#define PUMP_ADJUST_EXIT_TO_PREV_PAGE_ENABLE   (0)
#endif

/// 泵奶暂停时：点击刺激/吸乳模式键切换模式并启动泵奶
#ifndef PUMP_PAUSE_MODE_KEY_START_ENABLE
#define PUMP_PAUSE_MODE_KEY_START_ENABLE       (1)
#endif

/// 泵奶暂停静默超时：由 PM 倒计时触发结算后再关机（与 1.2h FINISH_TIMEOUT 区分）
#ifndef PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
#define PUMP_PAUSE_IDLE_TIMEOUT_ENABLE         (1)
#endif
#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
#ifndef PUMP_PAUSE_IDLE_TIMEOUT_MS
#define PUMP_PAUSE_IDLE_TIMEOUT_MS             PM_SUSPEND_TIME_S
#endif
#define PUMP_PAUSE_IDLE_LOG_INTERVAL_MS        (3000) /// 倒计时日志间隔
/**
 * 暂停结算页方案：
 * 方案1（默认）：按键灯全灭，展示完成后自动关机，按键不响应
 * 方案2：按键灯全亮，点击任意按键退出结算并执行对应功能；无按键则展示完成后仍自动关机
 */
#define PUMP_PAUSE_TIMEOUT_SCHEME_AUTO_OFF     (1)
#define PUMP_PAUSE_TIMEOUT_SCHEME_KEY_EXIT     (2)
#ifndef PUMP_PAUSE_TIMEOUT_SCHEME
#define PUMP_PAUSE_TIMEOUT_SCHEME              PUMP_PAUSE_TIMEOUT_SCHEME_AUTO_OFF
#endif
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
    PUMP_HDR_ACTION_PUMP_WILL_FINISH_EXIT, /// 韵律即将结束忽略退出
    PUMP_HDR_ACTION_MAX
} pump_ui_hdr_action_e;

/// 泵奶UI类型枚举
typedef enum {
    PUMP_UI_TYPE_NONE = 0,
    PUMP_UI_TYPE_HOME = 100,
    PUMP_UI_TYPE_RHYTHM_CONFIRM, /// 切换韵律确认
    PUMP_UI_TYPE_SETTINGS_SUCCESS, /// 模式切换成功
    PUMP_UI_TYPE_STOP_HOLD, /// 长按 power 停止泵奶确认
    PUMP_UI_TYPE_POWER_OFF_HOLD, /// 长按 power/start 关机确认
    PUMP_UI_TYPE_LEVEL_ADJUST, /// 单边力度调节
    PUMP_UI_TYPE_BOTH_LEVEL_ADJUST, /// 双边力度调节
    PUMP_UI_TYPE_SPEED_ADJUST, /// 速度调节
    PUMP_UI_TYPE_FINISH_TIMEOUT, /// 泵奶超时结算（1.2h）
    PUMP_UI_TYPE_FINISH_CHARGE_ERROR, /// 适配器不匹配结束提示
    PUMP_UI_TYPE_PAUSE_TIMEOUT, /// 暂停无操作超时结算（方案1→自动关机 / 方案2可按键退出）
    PUMP_UI_TYPE_WILL_FINISH, /// 韵律即将结束提示
    PUMP_UI_TYPE_LOW_POWER, /// 泵奶中低电量提示（3s 倒计时，可按键退出）
    PUMP_UI_TYPE_SEAL_LEAK, /// 重度漏气提醒（Check Fit / Tubing / Cup / 暂停）
    PUMP_UI_TYPE_MAX
} pump_ui_type_e;

typedef enum {
    PUMP_LEVEL_ADJUST_SIDE_LEFT = 0,
    PUMP_LEVEL_ADJUST_SIDE_RIGHT = 1,
} pump_level_adjust_side_e;

/// 漏气检测提示（泵奶首页）
typedef enum {
    PUMP_SEAL_TIP_NONE = 0,            /// 不显示，恢复波形
    PUMP_SEAL_TIP_CHECKING,            /// 检查中
    PUMP_SEAL_TIP_RESULT,              /// 检查结果（仅不漏气）
} pump_seal_tip_e;

/// 漏气检测结果（算法回调驱动）
typedef enum {
    PUMP_SEAL_RESULT_NONE = 0,         /// 尚未出结果：展示检测中
    PUMP_SEAL_RESULT_NO_LEAK,          /// 不漏气：检测中结束后展示通过文案 3s
    PUMP_SEAL_RESULT_SLIGHT,           /// 轻微漏气：收起检测中，不展示通过
    PUMP_SEAL_RESULT_SEVERE,           /// 重度漏气：首次变为该状态时跳转
} pump_seal_result_e;

/// 重度漏气提醒页（与 UI 四页对齐，由 ui_proc_pump_t.seal_leak_page_index 驱动）
typedef enum {
    PUMP_SEAL_LEAK_PAGE_CHECK_FIT = 0,     /// Check Fit
    PUMP_SEAL_LEAK_PAGE_CHECK_TUBING,      /// Check Tubing
    PUMP_SEAL_LEAK_PAGE_CHECK_CUP,         /// Check Cup
    PUMP_SEAL_LEAK_PAGE_PAUSE,             /// 漏气暂停页
    PUMP_SEAL_LEAK_PAGE_MAX
} pump_seal_leak_page_e;

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

/**
 * 泵奶窗口跨生命周期持久状态（不随 ui_proc_pump_t / 窗口销毁清零）
 * 运行期不清零，仅上下电（BSS 复位）后清除
 */
typedef struct {
    uint8_t rhythm_confirm_shown; /// 是否已展示过切换韵律确认页（上下电前一直有效）
} pump_ui_persist_t;

/// 泵奶UI结构体
typedef struct {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_entry_extra_t extra;
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
    uint8_t settings_success_style; /// settings success 文案样式 SETTINGS_SUCCESS_STYLE_*

    /// 长按停止确认
    uint8_t stop_hold_holding; /// 停止确认页是否处于按住倒计时
    /// 长按关机确认
    uint8_t power_off_hold_holding; /// 关机确认页是否处于按住倒计时
    uint8_t both_control_long_acted; /// 本次 L+R 长按已处理进入/退出，忽略同一次按住的后续 LONG_xS
    uint8_t low_power_ui_shown; /// 本次泵奶窗口已展示过低电页，电量恢复前不再重复弹出

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
    uint8_t will_finish_mode; /// 韵律即将结束模式
    int sub_mode_time_index; /// 调节页缓存的 mode step index，读写 setting 用
    uint32_t level_adjust_time; /// 力度/速度调节页最近操作时间（静默退出计时）
    uint32_t adjust_enter_time; /// 力度/速度调节页进入时间（按键退出防抖，与静默计时分离）
    uint32_t level_over_limit_time; /// 过大提示计时起点（有调节刷新）
    uint32_t level_inc_window_time; /// 连续加档窗口起点
    uint32_t sub_mode_switch_time; /// 子模式切换提醒计时（力度/速度页共用）

    /// 双边力度调节
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
    uint32_t blink_start_time; /// 上次闪烁翻转时间（相对半周期）

    /// 漏气检测提示（泵奶首页，pump_seal_tip_e）
    uint8_t seal_tip;
    /// 本轮检测窗口内最新结果 pump_seal_result_e
    uint8_t seal_result;
    uint32_t seal_check_time; /// 非 0 表示本轮启动已进入检测
    uint32_t seal_result_time; /// 通过文案起始时间
    uint32_t seal_log_time; /// 结果展示日志时间点
    /// 重度漏气提醒：本次进入泵奶窗口是否已跳转过；页索引 pump_seal_leak_page_e
    uint8_t seal_leak_shown;
    uint16_t seal_leak_page_index;

} ui_proc_pump_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 跨窗口持久状态（运行期不清零，仅上下电清除）
pump_ui_persist_t *pump_ui_persist_get(void);
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
/// 漏气检测：每次启动展示检测中，由算法结果切换文案 / 跳转
void pump_seal_check_start(ui_proc_pump_t *proc);
void pump_seal_check_stop(ui_proc_pump_t *proc);

/// 电源按键事件处理
void pump_power_key_event_handler(ui_proc_pump_t *proc);
/// 开始按键事件处理
void pump_start_key_event_handler(ui_proc_pump_t *proc);
/// 模式/韵律确认后：仅吸乳暂停态启动泵奶
void pump_start_pump_if_pause(ui_proc_pump_t *proc);
/// 启动/续抽且不回退导航（IDLE/PAUSE/FINISH → 运行，供成功页等覆盖层使用）
void pump_start_pump_keep_ui(ui_proc_pump_t *proc);

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
void pump_rhythm_confirm_start_key_handler(ui_proc_pump_t *proc);

/// 模式切换成功页
void show_pump_settings_success_ui(ui_proc_pump_t *proc, bool is_need_start_pump);
void pump_settings_success_ui_exit(ui_proc_pump_t *proc);

/// 长按 power 停止泵奶确认页
void show_pump_stop_hold_ui(ui_proc_pump_t *proc);
void pump_stop_hold_ui_exit(ui_proc_pump_t *proc);
void pump_stop_hold_dismiss(ui_proc_pump_t *proc);
void pump_stop_hold_key_long_2s_handler(ui_proc_pump_t *proc);
void pump_stop_hold_key_up_handler(ui_proc_pump_t *proc);

/// 长按 power/start 关机确认页
void show_pump_power_off_hold_ui(ui_proc_pump_t *proc);
void pump_power_off_hold_ui_exit(ui_proc_pump_t *proc);
void pump_power_off_hold_dismiss(ui_proc_pump_t *proc);
void pump_power_off_hold_key_long_2s_handler(ui_proc_pump_t *proc);
void pump_power_off_hold_key_up_handler(ui_proc_pump_t *proc);

/// 力度/速度调节页无操作 5s 静默退出：按键刷新计时
void pump_adjust_idle_touch(ui_proc_pump_t *proc);

/// 单边力度调节
void show_pump_level_adjust_ui(ui_proc_pump_t *proc, uint8_t is_left);
int pump_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase);
void pump_level_adjust_ui_exit(ui_proc_pump_t *proc);
void pump_level_adjust_ui_dismiss(ui_proc_pump_t *proc);

/// 双边力度调节（同时短按 L+R 进入，非限制态无操作 5s 静默退出）
void show_pump_both_level_adjust_ui(ui_proc_pump_t *proc);
int pump_both_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase);
void pump_both_level_adjust_ui_exit(ui_proc_pump_t *proc);
void pump_both_level_adjust_ui_dismiss(ui_proc_pump_t *proc);

/// 速度调节（无操作 5s 静默退出）
void show_pump_speed_adjust_ui(ui_proc_pump_t *proc);
int pump_speed_adjust_handler(ui_proc_pump_t *proc, bool is_increase);
void pump_speed_adjust_ui_exit(ui_proc_pump_t *proc);
void pump_speed_adjust_ui_dismiss(ui_proc_pump_t *proc);

/// 泵奶超时结算页（1.2h → 首页结果页）
void show_pump_finish_timeout_ui(ui_proc_pump_t *proc);
void pump_finish_timeout_ui_exit(ui_proc_pump_t *proc);

/// 适配器不匹配结束提示页（3s 倒计时 → 首页结果页；倒计时中适配器恢复则回泵奶首页）
void show_pump_finish_charge_error_ui(ui_proc_pump_t *proc);
void pump_finish_charge_error_ui_exit(ui_proc_pump_t *proc);
void pump_finish_charge_error_ui_dismiss(ui_proc_pump_t *proc);

#if PUMP_PAUSE_IDLE_TIMEOUT_ENABLE
/// 暂停无操作超时结算页（方案1：自动关机；方案2：可按键退出）
void show_pump_pause_timeout_ui(ui_proc_pump_t *proc);
void pump_pause_timeout_ui_exit(ui_proc_pump_t *proc);
#if PUMP_PAUSE_TIMEOUT_SCHEME == PUMP_PAUSE_TIMEOUT_SCHEME_KEY_EXIT
/// 方案2：退出暂停结算页并回到泵奶首页（保持暂停态，供后续按键落地）
void pump_pause_timeout_ui_dismiss(ui_proc_pump_t *proc);
#endif
/// PM 静默超时或窗口本地触发：进入暂停结算页（结算完成后自动关机）
void pump_pause_settle_trigger(ui_proc_pump_t *proc);
/// 暂停态生命周期占位（不再持休眠/关机锁，也不再本地倒计时）
void pump_pause_idle_start(ui_proc_pump_t *proc);
void pump_pause_idle_touch(ui_proc_pump_t *proc);
void pump_pause_idle_stop(ui_proc_pump_t *proc);
#endif

/// 韵律即将结束提示页（短按 power 切吸吮并回首页；5s 超时仅回首页不切模式；其它按键不响应）
void show_pump_will_finish_ui(ui_proc_pump_t *proc);
void pump_will_finish_ui_exit(ui_proc_pump_t *proc);
void pump_will_finish_ui_dismiss(ui_proc_pump_t *proc);

/// 重度漏气提醒：检测到后暂停泵奶并进入 Check Fit 首页；1-3 页禁短按 power 启动，第 4 页解除
void show_pump_seal_leak_ui(ui_proc_pump_t *proc);
void pump_seal_leak_ui_exit(ui_proc_pump_t *proc);
int pump_seal_leak_page_adjust(ui_proc_pump_t *proc, bool is_left);
void pump_seal_leak_resume(ui_proc_pump_t *proc);
/// 1-3 页为 true：禁用短按 power 启动吸奶
bool pump_seal_leak_restrict_keys(const ui_proc_pump_t *proc);
/// 重度漏气：本窗口尚未展示过则跳转重度 UI
void pump_seal_leak_jump_severe(ui_proc_pump_t *proc);

/// 泵奶中低电量提示页（3s 倒计时；按任意按键退出并执行对应功能；超时回首页，若覆盖了 30 分钟提醒则恢复）
void show_pump_low_power_ui(ui_proc_pump_t *proc);
void pump_low_power_ui_exit(ui_proc_pump_t *proc);
void pump_low_power_ui_dismiss(ui_proc_pump_t *proc);

/// 漏气检测初始化
void pump_seal_leak_init(ui_proc_pump_t *proc);
/// 漏气检测退出
void pump_seal_leak_exit(ui_proc_pump_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


