#ifndef __WINDOW_SCREEN_CHARGE_H__
#define __WINDOW_SCREEN_CHARGE_H__

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

/// 充电动画频率
#if IS_SUPPORT_LVGL
#define CHARGE_ANIMATION_FREQ                    (25)
#else
#define CHARGE_ANIMATION_FREQ                    (5)
#endif

typedef enum {
    CHARGE_HDR_EXIT_CHARGE_POLICY = 100, /// 退出充电策略
    CHARGE_HDR_CHARGE_SCREEN_ON,
    CHARGE_HDR_ACTION_MAX
} charge_ui_hdr_action_e;

/// 充电动UI类型枚举
typedef enum {
    CHARGE_UI_TYPE_NONE = 0,
    CHARGE_UI_TYPE_CHARGING = 100,
    CHARGE_UI_TYPE_ADAPTER,
    CHARGE_UI_TYPE_MAX
} charge_ui_type_e;

/// 充电动UI结构体
typedef struct {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    battery_manager_context_t *battery;
    key_input_context_t *key;
    led_manager_context_t *led;
    airplane_mode_context_t *airplane;
    power_manager_context_t *pm;
    ble_manager_context_t *ble;
    resource_manager_context_t *res;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic;
    uint8_t window_run_state; /// WM是否运行.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. charge_ui_type_e
} ui_proc_charge_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_charge_adapter_ui(ui_proc_charge_t *proc);
void stop_charge_common_ui(ui_proc_charge_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


