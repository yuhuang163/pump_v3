#ifndef __WINDOW_SCREEN_OTA_H__
#define __WINDOW_SCREEN_OTA_H__

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

/// ota动画频率
#if IS_SUPPORT_LVGL
#define OTA_ANIMATION_FREQ                 (25)
#else
#define OTA_ANIMATION_FREQ                 (5)
#endif

/// ota动画展示时间
#define OTA_ANIMATION_DISPLAY_TIME         (2000) /// 2s
#define OTA_RESULT_DISPLAY_TIME            (5000) /// 5s

/// ota UI类型枚举
typedef enum {
    OTA_UI_TYPE_NONE = 0,
    OTA_UI_TYPE_UPDATE,
    OTA_UI_TYPE_MAX
} ota_ui_type_e;

typedef enum {
    OTA_UI_STATUS_DOWNLOADING = 0,
    OTA_UI_STATUS_ERROR = 1,
    OTA_UI_STATUS_COMPLETE = 2,
} ota_ui_status_e;

/// ota UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    power_manager_context_t *pm;
    ui_manager_context_t *ui;
    led_manager_context_t *led;
    ble_manager_context_t *ble;
    ota_manager_context_t *ota;
    key_input_context_t *key;
    battery_manager_context_t *battery;
    airplane_mode_context_t *airplane;
    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. ota_ui_type_e
    uint8_t ota_status; /// OTA状态. ota_status_e
} ui_proc_ota_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_ota_ui_animation(ui_proc_ota_t *proc);
void stop_ota_ui_animation(ui_proc_ota_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


