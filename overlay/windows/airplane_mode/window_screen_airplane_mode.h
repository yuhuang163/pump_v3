#ifndef __WINDOW_SCREEN_AIRPLANE_MODE_H__
#define __WINDOW_SCREEN_AIRPLANE_MODE_H__

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

/// 飞行模式动画频率
#if IS_SUPPORT_LVGL
#define AIRPLANE_MODE_ANIMATION_FREQ                    (25)
#else
#define AIRPLANE_MODE_ANIMATION_FREQ                    (5)
#endif
/// 飞行模式动画更新间隔
#define AIRPLANE_MODE_ANIMATION_UPDATE_INTERVAL         (1000)
/// 飞行模式超时时间
#define AIRPLANE_MODE_TIMEOUT_TIME                      (2 * 60 * 1000) /// 2分钟

/// 飞行模式UI动作枚举
typedef enum {
    AIRPLANE_MODE_HDR_ACTION_REFRESH_CHARGE_UI = 0,
    AIRPLANE_MODE_HDR_ACTION_MAX
} airplane_mode_ui_hdr_action_e;

/// 飞行模式UI类型枚举
typedef enum {
    AIRPLANE_MODE_UI_TYPE_NONE = 0,
    AIRPLANE_MODE_UI_TYPE_HOME,
    AIRPLANE_MODE_UI_TYPE_MAX
} airplane_mode_ui_type_e;

/// 飞行模式UI结构体
typedef struct ui_proc_airplane_mode {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    battery_manager_context_t *battery;
    power_manager_context_t *pm;
    key_input_context_t *key;
    breast_pump_manager_context_t *pump;
    led_manager_context_t *led;
    airplane_mode_context_t *airplane;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. airplane_mode_ui_type_e
} ui_proc_airplane_mode_t;

/// 飞行模式背光灯状态结构体
typedef struct {    
  uint8_t id; /// 背光灯索引
  uint8_t level; /// 背光灯状态
} airplane_mode_led_status_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_airplane_mode_ui(ui_proc_airplane_mode_t *proc);
void stop_airplane_mode_ui(ui_proc_airplane_mode_t *proc);
void airplane_mode_led_policy(ui_proc_airplane_mode_t *proc, uint8_t status);
void airplane_mode_pump_policy(ui_proc_airplane_mode_t *proc);
#ifdef __cplusplus
}
#endif

#endif 


