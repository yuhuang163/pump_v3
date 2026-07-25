#ifndef __WINDOW_SCREEN_SYSTEM_ABNORMAL_H__
#define __WINDOW_SCREEN_SYSTEM_ABNORMAL_H__

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
#include "abnormal_manager_api.h" // IWYU pragma: keep
#include "led_manager_api.h" // IWYU pragma: keep

/// 系统异常动画频率
#if IS_SUPPORT_LVGL
#define SYSTEM_ABNORMAL_ANIMATION_FREQ                    (25)
#else
#define SYSTEM_ABNORMAL_ANIMATION_FREQ                    (5)
#endif


/// 系统异常UI动作枚举
typedef enum {
    SYSTEM_ABNORMAL_HDR_ACTION_EXIT = 100, /// 退出异常窗口
    SYSTEM_ABNORMAL_HDR_ACTION_MAX
} system_abnormal_ui_hdr_action_e;

/// 系统异常UI类型枚举
typedef enum {
    SYSTEM_ABNORMAL_UI_TYPE_NONE = 0,
    SYSTEM_ABNORMAL_UI_TYPE_BATTERY_TEMP_HIGH = 100, /// 电池温度过高
    SYSTEM_ABNORMAL_UI_TYPE_STORAGE_FAILURE,          /// 存储芯片故障
    SYSTEM_ABNORMAL_UI_TYPE_COMMUNICATION_ERROR,      /// 外设通信丢失
    SYSTEM_ABNORMAL_UI_TYPE_MAX
} system_abnormal_ui_type_e;

/// 系统异常UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    abnormal_manager_context_t *abnormal;
    key_input_context_t *key;
    power_manager_context_t *pm;
    led_manager_context_t *led;
    battery_manager_context_t *battery;
    airplane_mode_context_t *airplane;
    resource_manager_context_t *res;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间
    uint16_t current_ui_type; /// 当前UI类型. system_abnormal_ui_type_e
    uint8_t window_run_state; /// WM是否运行
} ui_proc_system_abnormal_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_system_abnormal_ui(ui_proc_system_abnormal_t *proc);
void stop_system_abnormal_ui(ui_proc_system_abnormal_t *proc);

#ifdef __cplusplus
}
#endif

#endif
