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
#define AIRPLANE_MODE_ANIMATION_FREQ                    (12)
/// 飞行模式动画更新间隔
#define AIRPLANE_MODE_ANIMATION_UPDATE_INTERVAL         (1000)
/// 飞行模式超时时间
#define AIRPLANE_MODE_TIMEOUT_TIME                      (20 * 1000) // (2 * 60 * 1000) /// 2分钟

typedef enum {
    HDR_ACTION_REFRESH_CHARGE_UI = 0,
    HDR_ACTION_MAX
} airplane_mode_ui_hdr_action_e;

typedef enum {
    AIRPLANE_MODE_UI_TYPE_MAX
} airplane_mode_ui_type_e;

typedef struct ui_proc_airplane_mode {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t charging_status; /// 充电状态
    uint8_t battery_percent; /// 电池电量百分比
} ui_proc_airplane_mode_t;

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


