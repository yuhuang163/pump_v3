#ifndef __WINDOW_SCREEN_HOME_H__
#define __WINDOW_SCREEN_HOME_H__

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

#define HOME_ANIMATION_FREQ                    (25)

/// 主界面绑定二维码展示时长
#define HOME_BIND_QRCODE_DISPLAY_TIMEOUT       (3000) /// 3s

/// 力度调节过大限制与提示（单边/双边共用，默认关闭）
#ifndef HOME_LEVEL_OVER_LIMIT_ENABLE
#define HOME_LEVEL_OVER_LIMIT_ENABLE           (0)
#endif
#if HOME_LEVEL_OVER_LIMIT_ENABLE
/// 单边/双边力度连续加档限制窗口时长
#define HOME_LEVEL_OVER_LIMIT_WINDOW_MS        (2000) /// 2s
/// 调节过大后静默，进度条开始回退
#define HOME_LEVEL_OVER_LIMIT_ARC_ROLLBACK_MS  (1000) /// 1s
/// 调节过大后静默，过大提醒文案消失
#define HOME_LEVEL_OVER_LIMIT_TIP_CLEAR_MS     (2000) /// 2s
#define HOME_LEVEL_OVER_LIMIT_MAX_STEP         (3)    /// 窗口内最多加 3 档
#endif
/// 子模式切换提醒时长
#define HOME_SUB_MODE_SWITCH_TIMEOUT_MS        (3000) /// 3s

/// 模式选择页无操作静默回首页（page_index 非 0 时计时，超时仅复位预览；
/// 不更换工作模式，模式落地仅由 power 确认触发）
#ifndef HOME_MODE_SELECT_IDLE_EXIT_ENABLE
#define HOME_MODE_SELECT_IDLE_EXIT_ENABLE      (1)
#endif
#define HOME_MODE_SELECT_IDLE_TIMEOUT_MS       (5000) /// 5s

/// 力度调节页：非力度限制态无操作静默退出
#define HOME_LEVEL_ADJUST_IDLE_TIMEOUT_MS      (5000) /// 5s
/// 速度调节页：无操作静默退出
#define HOME_SPEED_ADJUST_IDLE_TIMEOUT_MS      (5000) /// 5s

/// 力度/速度调节页：进入后再次点击对应按键退出的防抖保护
#ifndef HOME_ADJUST_KEY_EXIT_PROTECT_ENABLE
#define HOME_ADJUST_KEY_EXIT_PROTECT_ENABLE    (1)
#endif
#define HOME_ADJUST_KEY_EXIT_PROTECT_MS        (100) /// 100ms

/// 首页主提示文案类型（ui_proc_home_t.hint_low_bat）
#define HOME_HINT_TYPE_NORMAL                  (0) /// Press to start pumping
#define HOME_HINT_TYPE_LOW_BAT                 (1) /// BATTERY_STATE_LOW_POWER_RUN
#define HOME_HINT_TYPE_FORBID_START            (2) /// BATTERY_STATE_FORBID_START


/// 力度/速度调节页退出：1=回到上一页（保留 page_index），0=回本窗口首页（page_index 清零）
#ifndef HOME_ADJUST_EXIT_TO_PREV_PAGE_ENABLE
#define HOME_ADJUST_EXIT_TO_PREV_PAGE_ENABLE   (0)
#endif

/**
 * 待机页（HOME_IDLE）方案：
 * 方案1（默认）：进入熄灭按键灯，仅短按 power 退出，其它按键不响应；
 *               非按键解除（PM SYS_IDLE_RELEASE）不退待机，仍在 IDLE 则保持熄灯
 * 方案2：进入不灭按键灯，短按任意按键（含旋钮）仅退出待机回首页，不执行按键功能；
 *        非按键解除时若仍为 HOME_IDLE 则退出回 HOME
 * 方案3：进入不灭按键灯，短按任意按键先退出待机，再执行与 HOME 页一致的功能；
 *        非按键解除时若仍为 HOME_IDLE 则退出回 HOME；
 *        按键已 prepare_action / 进入功能页时不覆盖（功能页优先）
 * 三种方案下长按 power 均可关机
 */
#define HOME_IDLE_SCHEME_POWER_EXIT            (1)
#define HOME_IDLE_SCHEME_ANY_KEY_EXIT          (2)
#define HOME_IDLE_SCHEME_ANY_KEY_ACTION        (3)
#ifndef HOME_IDLE_SCHEME
#define HOME_IDLE_SCHEME                       HOME_IDLE_SCHEME_ANY_KEY_EXIT
#endif

/// 子模式同步场景（力度/速度页共用）
typedef enum {
    HOME_SUB_MODE_SYNC_LEVEL = 0,      /// 单边力度
    HOME_SUB_MODE_SYNC_BOTH_LEVEL,     /// 双边力度
    HOME_SUB_MODE_SYNC_SPEED,          /// 速度
} home_sub_mode_sync_e;

/// 泵奶结果显示超时时间
#define HOME_PUMP_RESULT_DISPLAY_TIMEOUT       (3000) /// 3s

/// 模式切换成功页展示时长
#define HOME_SETTINGS_SUCCESS_DISPLAY_TIMEOUT  (1500) /// 1.5s

/// 夜灯调节界面动画展示时间
#define HOME_NIGHT_LIGHT_ADJUST_SYNC_TIMEOUT   (3000) /// 3s

/// UTC 校准失效时间
#define HOME_UTC_CALIB_EXPIRE_INTERVAL_SEC     (20 * 24 * 3600)

/// 主界面状态闪烁半周期(ms)，与泵奶界面同频
#define HOME_BLINK_PERIOD                      (500)

/// 主界面UI类型枚举
typedef enum {
    HOME_UI_TYPE_NONE = 0,
    HOME_UI_TYPE_HOME,
    HOME_UI_TYPE_HOME_IDLE,
    HOME_UI_TYPE_PUMP_RESULT,
    HOME_UI_TYPE_SETTINGS_SUCCESS, /// 模式切换成功
    HOME_UI_TYPE_NIGHT_LIGHT_ADJUST, /// 夜灯调节
    HOME_UI_TYPE_POWER_OFF_HOLD, /// 长按 power 关机确认
    HOME_UI_TYPE_LEVEL_ADJUST, /// 单边力度调节
    HOME_UI_TYPE_BOTH_LEVEL_ADJUST, /// 双边力度调节
    HOME_UI_TYPE_SPEED_ADJUST, /// 速度调节
    HOME_UI_TYPE_MAX
} home_ui_type_e;

typedef enum {
    HOME_LEVEL_ADJUST_SIDE_LEFT = 0,
    HOME_LEVEL_ADJUST_SIDE_RIGHT = 1,
} home_level_adjust_side_e;

typedef enum {
    HOME_MODE_PAGE_INDEX_NONE = 0,                   /// 无模式选择
    HOME_MODE_PAGE_INDEX_STIMULATION_SELECT = 1,     /// 刺激模式选择
    HOME_MODE_PAGE_INDEX_EXPRESSION_SELECT = 2,      /// 吸乳模式选择
    HOME_MODE_PAGE_INDEX_MILK_BOOST_SELECT = 3,      /// 奶量提升选择
    HOME_MODE_PAGE_INDEX_POWER_PUMPING_SELECT = 4,   /// Power Pumping选择
    HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT = 5,       /// 个性化选择
} home_mode_page_index_e;

/**
 * 子页返回策略（show 时 push，dismiss 时选策略）
 * - HISTORY：弹出栈顶，恢复进入前 UI（力度/速度/首页+page_index）
 * - HOME_KEEP_PAGE：回窗口首页，保留/恢复 page_index（模式选择页）
 * - HOME_RESET_PAGE：回窗口首页，page_index 清零
 */
typedef enum {
    HOME_UI_NAV_BACK_HISTORY = 0,
    HOME_UI_NAV_BACK_HOME_KEEP_PAGE,
    HOME_UI_NAV_BACK_HOME_RESET_PAGE,
} home_ui_nav_back_e;

#define HOME_UI_NAV_DEPTH_MAX                       (2)

typedef struct {
    uint16_t ui_type;       /// home_ui_type_e
    uint16_t page_index;    /// ui_mode_page_index
    uint8_t level_side;     /// 单边力度侧（仅 LEVEL_ADJUST 有意义）
} home_ui_nav_frame_t;

typedef struct {
    home_ui_nav_frame_t stack[HOME_UI_NAV_DEPTH_MAX];
    uint8_t depth;
    uint8_t restoring;      /// nav_back 恢复中，避免 show 再次 push
} home_ui_nav_t;

/// 主界面UI结构体
typedef struct {
    window_context_t w_ctx;
    window_entry_extra_t extra;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    ble_manager_context_t *ble;
    battery_manager_context_t *battery;
    key_input_context_t *key;
    led_manager_context_t *led;
    breast_pump_manager_context_t *pump;
    motor_manager_context_t *motor;
    airplane_mode_context_t *airplane_mode;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t window_run_state; /// WM是否运行.
    uint16_t current_ui_type; /// 当前UI类型. home_ui_type_e
    uint16_t popup_ui_type; /// 当前弹窗类型. home_popup_ui_type_e
    
    /// 模式相关
    uint16_t ui_mode_page_index; /// 当前UI模式页面索引.
    uint32_t mode_select_idle_time; /// 模式选择页无操作计时起点

    /// 档位调节相关
    pump_setting_t setting; /// 当前泵奶设置信息.
    uint8_t level_adjust_side; /// 单边调节侧 home_level_adjust_side_e
    uint8_t level_value; /// 实际/展示力度值（过大提示时不变）
    uint8_t level_arc_level; /// 圆弧动画目标力度（可与 level_value 分离）
    uint8_t level_over_limit; /// 力度过大提示中
    uint8_t level_inc_window_base; /// 连续加档窗口起始力度
    uint8_t sub_mode_type; /// 当前子模式 pump_mode_type_e（力度/速度页共用）
    uint8_t sub_mode_switch; /// 子模式切换提醒中（力度/速度页共用）
    int sub_mode_time_index; /// 调节页缓存的 mode step index，读写 setting 用
    uint8_t both_level_value[2]; /// 双边实际/展示力度（过大提示时不变）
    uint8_t both_level_arc[2]; /// 双边圆弧动画目标力度
    uint8_t both_level_over_limit; /// 双边力度过大提示中
    uint8_t both_level_inc_steps; /// 双边连续加档窗口内已加档次数
    uint32_t level_adjust_time; /// 力度/速度调节页最近操作时间（静默退出计时）
    uint32_t adjust_enter_time; /// 力度/速度调节页进入时间（按键退出防抖，与静默计时分离）
    uint32_t level_over_limit_time; /// 过大提示计时起点（有调节刷新）
    uint32_t level_inc_window_time; /// 连续加档窗口起点
    uint32_t sub_mode_switch_time; /// 子模式切换提醒计时（力度/速度页共用）
    uint32_t both_level_over_limit_time; /// 双边过大提示计时
    uint32_t both_level_inc_window_time; /// 双边连续加档窗口起点

    /// 夜灯调节相关
    uint8_t night_light_gear; /// 夜灯调节亮度等级
    uint32_t pump_record_time;

    /// 灯光相关
    uint8_t home_light_status; /// 主界面灯光状态.

    /// 长按关机确认
    uint8_t power_off_hold_holding; /// 关机确认页是否处于按住倒计时
    uint8_t both_control_long_acted; /// 本次 L+R 长按已处理进入/退出，忽略同一次按住的后续 LONG_xS

    /// 电池状态（由电池回调更新，首页提示文案读取，不轮询电量）
    uint8_t hint_low_bat; /// HOME_HINT_TYPE_*：0默认 / 1低电可跑 / 2禁止启动

    /// 子页导航栈（回历史页 / 回首页）
    home_ui_nav_t ui_nav;
} ui_proc_home_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc);
void start_home_idle_ui_animation(ui_proc_home_t *proc);
/// 待机页退出回首页（按键灯全亮）；退出触发见 HOME_IDLE_SCHEME
void home_idle_ui_dismiss(ui_proc_home_t *proc);
#if HOME_IDLE_SCHEME == HOME_IDLE_SCHEME_ANY_KEY_ACTION
/// 方案3：退出待机但不刷首页，供随后直接进入力度/速度等功能页，避免闪 HOME
void home_idle_ui_prepare_action(ui_proc_home_t *proc);
#endif
void stop_home_common_ui(ui_proc_home_t *proc);
/// 子页导航：进入子页前 push；退出时按策略 back
void home_ui_nav_push(ui_proc_home_t *proc);
void home_ui_nav_clear(ui_proc_home_t *proc);
void home_ui_nav_back(ui_proc_home_t *proc, home_ui_nav_back_e policy);

/// 模式选择界面按键处理
int home_mode_adjust_handler(ui_proc_home_t *proc, bool is_left);
int home_mode_confirm_handler(ui_proc_home_t *proc);
int home_mode_set_handler(ui_proc_home_t *proc, uint8_t mode_index);
void home_mode_page_index_clamp(ui_proc_home_t *proc);
/// 模式选择页无操作自动回首页：按键刷新计时 / 超时检测
void home_mode_select_idle_touch(ui_proc_home_t *proc);
void home_mode_select_idle_timeout_handler(ui_proc_home_t *proc);

/// 力度/速度调节页无操作 5s 静默退出：按键刷新计时
void home_adjust_idle_touch(ui_proc_home_t *proc);

/// 单边力度调节
void show_home_level_adjust_ui(ui_proc_home_t *proc, uint8_t is_left);
int home_pump_level_adjust_handler(ui_proc_home_t *proc, bool is_increase);
void home_level_adjust_ui_exit(ui_proc_home_t *proc);
void home_level_adjust_ui_dismiss(ui_proc_home_t *proc);

/// 双边力度调节（非限制态无操作 5s 静默退出；L+R 短按进入）
void show_home_both_level_adjust_ui(ui_proc_home_t *proc);
int home_pump_both_level_adjust_handler(ui_proc_home_t *proc, bool is_increase);
void home_both_level_adjust_ui_exit(ui_proc_home_t *proc);
void home_both_level_adjust_ui_dismiss(ui_proc_home_t *proc);

/// 速度调节（无操作 5s 静默退出）
void show_home_speed_adjust_ui(ui_proc_home_t *proc);
int home_pump_speed_adjust_handler(ui_proc_home_t *proc, bool is_increase);
void home_speed_adjust_ui_exit(ui_proc_home_t *proc);
void home_speed_adjust_ui_dismiss(ui_proc_home_t *proc);

/// 弹窗类型更新
void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type);

/// 子模式（刺激/吸乳/混合/放松）同步：力度/速度页共用
uint8_t home_sub_mode_type_get(ui_proc_home_t *proc);
void home_sub_mode_switch_clear(ui_proc_home_t *proc);
void home_sub_mode_switch_tick(ui_proc_home_t *proc);
void home_sub_mode_state_init(ui_proc_home_t *proc, uint8_t clamp_level);
void home_sub_mode_sync_tick(ui_proc_home_t *proc, home_sub_mode_sync_e sync);

/// 主界面UI切换
void home_ui_change_hdr_remove(ui_proc_home_t *proc);

/// 泵奶结果界面处理
void show_home_pump_result_ui(ui_proc_home_t *proc);
void home_pump_result_ui_exit(ui_proc_home_t *proc);

/// 启动泵奶（跳转泵奶窗口）；成功返回 0，电量不足等失败返回负数
int home_start_pump_handler(ui_proc_home_t *proc);
/// 跳转泵奶窗口并指定进入子页（如先展示 Success）
int home_start_pump_with_sub_ui(ui_proc_home_t *proc, pump_sub_ui_type_e sub_ui);

/// 模式切换成功页
void show_home_settings_success_ui(ui_proc_home_t *proc, bool is_need_start_pump);
void home_settings_success_ui_exit(ui_proc_home_t *proc);

/// 长按 power 关机确认页
void show_home_power_off_hold_ui(ui_proc_home_t *proc);
void home_power_off_hold_ui_exit(ui_proc_home_t *proc);
void home_power_off_hold_dismiss(ui_proc_home_t *proc);
void home_power_off_hold_key_long_2s_handler(ui_proc_home_t *proc);
void home_power_off_hold_key_up_handler(ui_proc_home_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


