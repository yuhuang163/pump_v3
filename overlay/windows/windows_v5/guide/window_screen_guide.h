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
#define GUIDE_ANIMATION_DISPLAY_TIME         (2000) /// 2s
#define GUIDE_BLINK_PERIOD                   (500)
/// 单边力度调节静止超时
#define GUIDE_LEVEL_ADJUST_IDLE_TIMEOUT_MS    (3000) /// 3s
/// 力度调节后，展示页面停留时间
#define GUIDE_LEVEL_ADJUST_IDLE_SHOW_TIME_MS  (200) /// 200ms
#define GUIDE_DEFAULT_LEFT_LEVEL              (12)
#define GUIDE_DEFAULT_RIGHT_LEVEL             (6)
#define GUIDE_LEVEL_MIN                       (0)
#define GUIDE_LEVEL_MAX                       (15)

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
    GUIDE_PAGE_PUMP_LEVEL_DOUBLE_ADJUST = 0, /// 引导泵奶力度双边调节
    GUIDE_PAGE_PUMP_LEVEL_SINGLE_SELECT = 1, /// 引导泵奶力度单边选择
    GUIDE_PAGE_PUMP_LEVEL_LEFT_ADJUST = 2, /// 引导泵奶力度左调节
    GUIDE_PAGE_PUMP_LEVEL_RIGHT_ADJUST = 3, /// 引导泵奶力度右调节
    GUIDE_PAGE_PUMP_MODE_SELECT = 4, /// 引导泵奶模式选择
    GUIDE_PAGE_PUMP_MODE_UNSELECT = 5, /// 引导泵奶未选择模式
    GUIDE_PAGE_MAX,
} guide_page_index_t;

typedef enum {
    GUIDE_MODE_SELECT_NONE = 0, /// 未选择模式
    GUIDE_MODE_SELECT_MODE = 1, /// 选择模式
    GUIDE_MODE_SELECT_BREASTFEEDING = 2, /// 选择哺乳
    GUIDE_MODE_SELECT_MAX,
} guide_mode_select_e;

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
    uint8_t state_blink_visible; /// 状态闪烁可见: 0: 隐藏相, 1: 显示相.
    uint32_t last_adjust_time; /// 上次力度调节时间
} ui_proc_guide_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_guide_ui_animation(ui_proc_guide_t *proc);
void stop_guide_ui_animation(ui_proc_guide_t *proc);
void guide_knob_event_handler(ui_proc_guide_t *proc, bool is_left);
void guide_key_event_handler(ui_proc_guide_t *proc, uint16_t keycode);
#ifdef __cplusplus
}
#endif

#endif 


